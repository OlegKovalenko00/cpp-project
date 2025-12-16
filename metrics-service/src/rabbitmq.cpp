#include "rabbitmq.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

RabbitMQConfig load_rabbitmq_config() {
    RabbitMQConfig config;

    const char* host = std::getenv("RABBITMQ_HOST");
    const char* port = std::getenv("RABBITMQ_PORT");
    const char* user = std::getenv("RABBITMQ_USER");
    const char* password = std::getenv("RABBITMQ_PASSWORD");
    const char* vhost = std::getenv("RABBITMQ_VHOST");

    config.host = host ? host : "localhost";
    config.port = port ? std::stoi(port) : 5672;
    config.user = user ? user : "guest";
    config.password = password ? password : "guest";
    config.vhost = vhost ? vhost : "/";

    // Default queues matching api-service
    config.queues = {"page_views", "clicks", "performance_events", "error_events", "custom_events"};

    return config;
}

RabbitMQConsumer::RabbitMQConsumer(const RabbitMQConfig& config) : config_(config) {
}

RabbitMQConsumer::~RabbitMQConsumer() {
    stop();
    disconnect();
}

bool RabbitMQConsumer::checkRpcReply(const char* context) {
    amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn_);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "[RabbitMQ] " << context << " failed" << std::endl;
        return false;
    }
    return true;
}

void RabbitMQConsumer::disconnect() {
    connected_ = false;
    if (conn_) {
        amqp_channel_close(conn_, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(conn_, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn_);
    }
    conn_ = nullptr;
    socket_ = nullptr;
}

bool RabbitMQConsumer::reconnect() {
    disconnect();
    if (!connect()) {
        return false;
    }
    if (callback_) {
        subscribe(callback_);
    }
    return true;
}

bool RabbitMQConsumer::connect() {
    std::cout << "RabbitMQ connecting to: " << config_.host << ":" << config_.port << std::endl;

    disconnect();

    conn_ = amqp_new_connection();
    if (!conn_) {
        std::cerr << "[RabbitMQ] Failed to create connection" << std::endl;
        return false;
    }

    socket_ = amqp_tcp_socket_new(conn_);
    if (!socket_) {
        std::cerr << "[RabbitMQ] Failed to create TCP socket" << std::endl;
        return false;
    }

    int status = amqp_socket_open(socket_, config_.host.c_str(), config_.port);
    if (status != AMQP_STATUS_OK) {
        std::cerr << "[RabbitMQ] Failed to open socket: " << amqp_error_string2(status)
                  << std::endl;
        return false;
    }

    amqp_rpc_reply_t reply =
        amqp_login(conn_, config_.vhost.c_str(), 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                   config_.user.c_str(), config_.password.c_str());
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "[RabbitMQ] Login failed" << std::endl;
        return false;
    }

    amqp_channel_open(conn_, 1);
    if (!checkRpcReply("Opening channel")) {
        return false;
    }

    // Объявляем очереди
    for (const auto& queue : config_.queues) {
        amqp_queue_declare(conn_, 1, amqp_cstring_bytes(queue.c_str()),
                           0, // passive
                           1, // durable
                           0, // exclusive
                           0, // auto_delete
                           amqp_empty_table);
        if (!checkRpcReply("Declaring queue")) {
            std::cerr << "[RabbitMQ] Failed to declare queue: " << queue << std::endl;
        } else {
            std::cout << "[RabbitMQ] Queue declared: " << queue << std::endl;
        }
    }

    connected_ = true;
    std::cout << "[RabbitMQ] Connected to " << config_.host << ":" << config_.port << std::endl;
    return true;
}

void RabbitMQConsumer::subscribe(MessageCallback callback) {
    callback_ = callback;

    // Подписываемся на все очереди
    for (const auto& queue : config_.queues) {
        amqp_basic_consume(conn_, 1, amqp_cstring_bytes(queue.c_str()),
                           amqp_empty_bytes, // consumer_tag
                           0,                // no_local
                           0,                // no_ack (we will ack manually)
                           0,                // exclusive
                           amqp_empty_table);
        if (!checkRpcReply("Starting consumer")) {
            std::cerr << "[RabbitMQ] Failed to start consuming from: " << queue << std::endl;
        } else {
            std::cout << "[RabbitMQ] Subscribed to queue: " << queue << std::endl;
        }
    }
}

void RabbitMQConsumer::consumeLoop() {
    std::cout << "[RabbitMQ] Consumer loop started" << std::endl;

    while (running_) {
        if (!connected_ || !conn_) {
            if (!reconnect()) {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }
            std::cout << "[RabbitMQ] Reconnected after disconnect" << std::endl;
        }

        amqp_envelope_t envelope;
        amqp_maybe_release_buffers(conn_);

        struct timeval timeout = {0, 100000}; // 100ms timeout
        amqp_rpc_reply_t reply = amqp_consume_message(conn_, &envelope, &timeout, 0);

        if (reply.reply_type == AMQP_RESPONSE_NORMAL) {
            std::string routing_key(static_cast<char*>(envelope.routing_key.bytes),
                                    envelope.routing_key.len);
            std::string body(static_cast<char*>(envelope.message.body.bytes),
                             envelope.message.body.len);

            std::cout << "[RabbitMQ] Received message from queue '" << routing_key << "'"
                      << std::endl;
            std::cout << "[RabbitMQ] Message body: " << body << std::endl;

            try {
                if (callback_) {
                    callback_(routing_key, body);
                }
                amqp_basic_ack(conn_, 1, envelope.delivery_tag, 0);
                std::cout << "[RabbitMQ] Message acknowledged" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[RabbitMQ] Error processing message: " << e.what() << std::endl;
                amqp_basic_reject(conn_, 1, envelope.delivery_tag, 1); // requeue
            }

            amqp_destroy_envelope(&envelope);
        } else if (reply.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION &&
                   reply.library_error == AMQP_STATUS_TIMEOUT) {
            // Timeout - это нормально, просто продолжаем
            continue;
        } else if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
            std::cerr << "[RabbitMQ] Consumer error, reconnecting..." << std::endl;
            if (!reconnect()) {
                std::cerr << "[RabbitMQ] Reconnect failed, will retry..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            } else {
                std::cout << "[RabbitMQ] Reconnected and re-subscribed" << std::endl;
            }
        }
    }

    std::cout << "[RabbitMQ] Consumer loop stopped" << std::endl;
}

void RabbitMQConsumer::start() {
    running_ = true;
    consumer_thread_ = std::thread([this]() {
        std::cout << "[RabbitMQ] Consumer started on queues: ";
        for (size_t i = 0; i < config_.queues.size(); ++i) {
            std::cout << config_.queues[i];
            if (i < config_.queues.size() - 1)
                std::cout << ", ";
        }
        std::cout << std::endl;

        consumeLoop();
    });
}

void RabbitMQConsumer::stop() {
    running_ = false;
    if (consumer_thread_.joinable()) {
        consumer_thread_.join();
    }
}
