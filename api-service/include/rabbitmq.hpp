#pragma once

#include <string>
#include <mutex>
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

private:
    // rabbitmq-c connection is not thread-safe, guard socket writes
    std::mutex mutex_;
    std::string host_;
    int port_;
    std::string username_;
    std::string password_;
    std::string vhost_;
    
    amqp_connection_state_t conn_ = nullptr;
    amqp_socket_t* socket_ = nullptr;
    bool connected_ = false;
    
    bool checkRpcReply(const char* context);
};
