#pragma once

#include <string>
#include <amqp.h>
#include <amqp_tcp_socket.h>

class RabbitMQ {
public:
    RabbitMQ(const std::string& host, int port, const std::string& username,
             const std::string& password, const std::string& vhost);
    ~RabbitMQ();

    bool connect();
    bool publish(const std::string& exchange, const std::string& routing_key, const std::string& message);
    bool isConnected() const { return connected_; }

    // Асинхронная публикация
    void start_async_publisher();
    void stop_async_publisher();
    void async_publish(const std::string& exchange, const std::string& routing_key, const std::string& message);

private:
    std::string host_;
    int port_;
    std::string username_;
    std::string password_;
    std::string vhost_;
    
    amqp_connection_state_t conn_ = nullptr;
    amqp_socket_t* socket_ = nullptr;
    bool connected_ = false;
    
    bool checkRpcReply(const char* context);

    // Для асинхронной публикации
    std::queue<std::tuple<std::string, std::string, std::string>> publish_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::thread publisher_thread_;
    std::atomic<bool> running_{false};
};

