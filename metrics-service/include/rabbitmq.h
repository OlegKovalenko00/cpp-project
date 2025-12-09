#pragma once

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
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
    using MessageCallback =
        std::function<void(const std::string& queue, const std::string& message)>;

    RabbitMQConsumer(const RabbitMQConfig& config);
    ~RabbitMQConsumer();

    bool connect();
    void subscribe(MessageCallback callback);
    void start();
    void stop();
    bool isConnected() const {
        return connected_;
    }

  private:
    bool checkRpcReply(const char* context);
    void consumeLoop();

    RabbitMQConfig config_;
    MessageCallback callback_;
    std::atomic<bool> running_{false};
    std::atomic<bool> connected_{false};
    std::thread consumer_thread_;

    amqp_connection_state_t conn_ = nullptr;
    amqp_socket_t* socket_ = nullptr;
};
