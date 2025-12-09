#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

struct RabbitMQConfig {
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string vhost;
    std::vector<std::string> queues;
};

RabbitMQConfig load_rabbitmq_config();

class RabbitMQConsumer {
public:
    // Callback receives queue name and message body
    using MessageCallback = std::function<void(const std::string& queue, const std::string& message)>;

    RabbitMQConsumer(const RabbitMQConfig& config);
    ~RabbitMQConsumer();

    void connect();
    void subscribe(MessageCallback callback);
    void start();
    void stop();

private:
    RabbitMQConfig config_;
    std::atomic<bool> running_{false};
    std::thread consumer_thread_;
    
    class Impl;
    std::unique_ptr<Impl> impl_;
};
