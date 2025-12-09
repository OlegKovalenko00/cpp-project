#include "rabbitmq.h"
#include <amqpcpp.h>
#include <amqpcpp/linux_tcp.h>
#include <event2/event.h>
#include <iostream>
#include <cstdlib>

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

class LibEventHandler : public AMQP::TcpHandler {
public:
    LibEventHandler(struct event_base* evbase) : evbase_(evbase) {}

    void onAttached(AMQP::TcpConnection* connection) override {}
    
    void onConnected(AMQP::TcpConnection* connection) override {
        std::cout << "RabbitMQ connected" << std::endl;
    }
    
    void onReady(AMQP::TcpConnection* connection) override {
        std::cout << "RabbitMQ ready" << std::endl;
    }
    
    void onError(AMQP::TcpConnection* connection, const char* message) override {
        std::cerr << "RabbitMQ error: " << message << std::endl;
    }
    
    void onClosed(AMQP::TcpConnection* connection) override {
        std::cout << "RabbitMQ connection closed" << std::endl;
    }

    void monitor(AMQP::TcpConnection* connection, int fd, int flags) override {
        if (fd_event_) {
            event_del(fd_event_);
            event_free(fd_event_);
            fd_event_ = nullptr;
        }

        if (flags == 0) return;

        short what = 0;
        if (flags & AMQP::readable) what |= EV_READ;
        if (flags & AMQP::writable) what |= EV_WRITE;

        fd_event_ = event_new(evbase_, fd, what | EV_PERSIST,
            [](evutil_socket_t fd, short what, void* arg) {
                auto* conn = static_cast<AMQP::TcpConnection*>(arg);
                if (what & EV_READ) conn->process(fd, AMQP::readable);
                if (what & EV_WRITE) conn->process(fd, AMQP::writable);
            }, connection);
        
        event_add(fd_event_, nullptr);
    }

private:
    struct event_base* evbase_;
    struct event* fd_event_ = nullptr;
};

class RabbitMQConsumer::Impl {
public:
    struct event_base* evbase_ = nullptr;
    std::unique_ptr<LibEventHandler> handler_;
    std::unique_ptr<AMQP::TcpConnection> connection_;
    std::unique_ptr<AMQP::TcpChannel> channel_;
};

RabbitMQConsumer::RabbitMQConsumer(const RabbitMQConfig& config)
    : config_(config), impl_(std::make_unique<Impl>()) {}

RabbitMQConsumer::~RabbitMQConsumer() {
    stop();
}

void RabbitMQConsumer::connect() {
    impl_->evbase_ = event_base_new();
    impl_->handler_ = std::make_unique<LibEventHandler>(impl_->evbase_);
    
    std::string address = "amqp://" + config_.user + ":" + config_.password + "@" + 
                          config_.host + ":" + std::to_string(config_.port) + config_.vhost;
    
    AMQP::Address amqp_address(address);
    
    impl_->connection_ = std::make_unique<AMQP::TcpConnection>(
        impl_->handler_.get(), amqp_address);
    impl_->channel_ = std::make_unique<AMQP::TcpChannel>(impl_->connection_.get());
}

void RabbitMQConsumer::subscribe(MessageCallback callback) {
    for (const auto& queue_name : config_.queues) {
        impl_->channel_->declareQueue(queue_name, AMQP::durable)
            .onSuccess([this, callback, queue_name](const std::string& name, uint32_t messagecount, uint32_t consumercount) {
                std::cout << "Queue '" << name << "' declared, messages: " << messagecount << std::endl;
                
                impl_->channel_->consume(queue_name)
                    .onReceived([this, callback, queue_name](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered) {
                        std::string body(message.body(), message.bodySize());
                        
                        try {
                            callback(queue_name, body);
                            impl_->channel_->ack(deliveryTag);
                        } catch (const std::exception& e) {
                            std::cerr << "Message processing error on queue " << queue_name << ": " << e.what() << std::endl;
                            impl_->channel_->reject(deliveryTag, true);
                        }
                    })
                    .onError([queue_name](const char* message) {
                        std::cerr << "Consume error on queue " << queue_name << ": " << message << std::endl;
                    });
            })
            .onError([queue_name](const char* message) {
                std::cerr << "Queue declare error for " << queue_name << ": " << message << std::endl;
            });
    }
}

void RabbitMQConsumer::start() {
    running_ = true;
    consumer_thread_ = std::thread([this]() {
        std::cout << "RabbitMQ consumer started on queues: ";
        for (size_t i = 0; i < config_.queues.size(); ++i) {
            std::cout << config_.queues[i];
            if (i < config_.queues.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
        
        while (running_) {
            event_base_loop(impl_->evbase_, EVLOOP_NONBLOCK);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}

void RabbitMQConsumer::stop() {
    running_ = false;
    if (consumer_thread_.joinable()) {
        consumer_thread_.join();
    }
    if (impl_->channel_) impl_->channel_.reset();
    if (impl_->connection_) impl_->connection_.reset();
    if (impl_->evbase_) {
        event_base_free(impl_->evbase_);
        impl_->evbase_ = nullptr;
    }
}
