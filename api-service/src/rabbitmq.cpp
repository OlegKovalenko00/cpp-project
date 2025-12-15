#include "rabbitmq.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>

namespace {

std::string getEnvStr(const char* name, const std::string& defaultValue) {
    const char* val = std::getenv(name);
    return val ? val : defaultValue;
}

int getEnvInt(const char* name, int defaultValue) {
    const char* val = std::getenv(name);
    return val ? std::stoi(val) : defaultValue;
}

} // namespace

RabbitMQ::RabbitMQ(const std::string& host, int port, const std::string& username,
                   const std::string& password, const std::string& vhost)
    : host_(host), port_(port), username_(username), password_(password), vhost_(vhost) {}

RabbitMQ::~RabbitMQ() {
    stop_async_publisher();
    if (conn_) {
        if (connected_) {
            amqp_channel_close(conn_, 1, AMQP_REPLY_SUCCESS);
            amqp_connection_close(conn_, AMQP_REPLY_SUCCESS);
        }
        amqp_destroy_connection(conn_);
        conn_ = nullptr;
    }
}

bool RabbitMQ::connect() {
    if (connected_) {
        return true;
    }

    if (conn_) {
        amqp_destroy_connection(conn_);
        conn_ = nullptr;
    }
    socket_ = nullptr;

    conn_ = amqp_new_connection();
    if (!conn_) {
        std::cerr << "[RabbitMQ] Failed to create connection" << std::endl;
        return false;
    }

    socket_ = amqp_tcp_socket_new(conn_);
    if (!socket_) {
        std::cerr << "[RabbitMQ] Failed to create TCP socket" << std::endl;
        amqp_destroy_connection(conn_);
        conn_ = nullptr;
        return false;
    }

    int status = amqp_socket_open(socket_, host_.c_str(), port_);
    if (status != AMQP_STATUS_OK) {
        std::cerr << "[RabbitMQ] Failed to open socket: " << amqp_error_string2(status) << std::endl;
        amqp_destroy_connection(conn_);
        conn_ = nullptr;
        socket_ = nullptr;
        return false;
    }

    amqp_rpc_reply_t reply = amqp_login(conn_, vhost_.c_str(), 0, 131072, 0,
                                         AMQP_SASL_METHOD_PLAIN,
                                         username_.c_str(), password_.c_str());
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "[RabbitMQ] Login failed" << std::endl;
        amqp_destroy_connection(conn_);
        conn_ = nullptr;
        socket_ = nullptr;
        return false;
    }

    amqp_channel_open(conn_, 1);
    if (!checkRpcReply("Opening channel")) {
        amqp_destroy_connection(conn_);
        conn_ = nullptr;
        socket_ = nullptr;
        return false;
    }

    const char* queues[] = {"page_views", "clicks", "performance_events", "error_events", "custom_events"};
    for (const char* queue : queues) {
        amqp_queue_declare(conn_, 1, amqp_cstring_bytes(queue),
                           0,
                           1,
                           0,
                           0,
                           amqp_empty_table);
        if (!checkRpcReply("Declaring queue")) {
            std::cerr << "[RabbitMQ] Failed to declare queue: " << queue << std::endl;
        } else {
            std::cout << "[RabbitMQ] Queue declared: " << queue << std::endl;
        }
    }

    connected_ = true;
    std::cout << "[RabbitMQ] Connected to " << host_ << ":" << port_ << std::endl;
    return true;
}

bool RabbitMQ::checkRpcReply(const char* context) {
    amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn_);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        std::cerr << "[RabbitMQ] " << context << " failed" << std::endl;
        return false;
    }
    return true;
}

bool RabbitMQ::publish(const std::string& exchange, const std::string& routing_key,
                       const std::string& message) {
    if (!connected_) {
        std::cerr << "[RabbitMQ] Not connected" << std::endl;
        return false;
    }

    amqp_bytes_t exchange_bytes = amqp_cstring_bytes(exchange.c_str());
    amqp_bytes_t routing_key_bytes = amqp_cstring_bytes(routing_key.c_str());
    amqp_bytes_t message_bytes;
    message_bytes.len = message.size();
    message_bytes.bytes = (void*)message.data();

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("application/json");
    props.delivery_mode = 2;

    int status = amqp_basic_publish(conn_, 1, exchange_bytes, routing_key_bytes,
                                    0, 0, &props, message_bytes);

    if (status != AMQP_STATUS_OK) {
        std::cerr << "[RabbitMQ] Publish failed: " << amqp_error_string2(status) << std::endl;
        return false;
    }

    std::cout << "[RabbitMQ] Published to queue '" << routing_key << "': "
              << message.substr(0, 100) << std::endl;
    return true;
}

void RabbitMQ::start_async_publisher() {
    if (running_) {
        return;
    }
    running_ = true;
    publisher_thread_ = std::thread([this]() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !publish_queue_.empty() || !running_; });

            while (!publish_queue_.empty()) {
                auto [exchange, routing_key, message] = publish_queue_.front();
                publish_queue_.pop();
                lock.unlock();

                if (!publish(exchange, routing_key, message)) {
                    std::cerr << "[RabbitMQ] Async publish failed for routing key '" << routing_key << "'" << std::endl;
                }

                lock.lock();
            }
        }
    });
}

void RabbitMQ::stop_async_publisher() {
    if (!running_) {
        return;
    }
    running_ = false;
    queue_cv_.notify_all();
    if (publisher_thread_.joinable()) {
        publisher_thread_.join();
    }
}

void RabbitMQ::async_publish(const std::string& exchange, const std::string& routing_key,
                             const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        publish_queue_.emplace(exchange, routing_key, message);
    }
    queue_cv_.notify_one();
}

RabbitMQ& getRabbitMQ() {
    static RabbitMQ rabbit(
        getEnvStr("RABBITMQ_HOST", "localhost"),
        getEnvInt("RABBITMQ_PORT", 5672),
        getEnvStr("RABBITMQ_USERNAME", "guest"),
        getEnvStr("RABBITMQ_PASSWORD", "guest"),
        getEnvStr("RABBITMQ_VHOST", "/"));

    static std::once_flag connect_flag;
    std::call_once(connect_flag, []() {
        if (!rabbit.connect()) {
            std::cerr << "[RabbitMQ] Failed to establish initial connection" << std::endl;
        }
    });

    return rabbit;
}
