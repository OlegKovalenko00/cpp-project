#pragma once

#include <string>
#include <amqpcpp.h>

class RabbitMQ {
public:
    RabbitMQ(const std::string& host, int port, const std::string& username,
             const std::string& password, const std::string& vhost);
    ~RabbitMQ();

    bool publish(const std::string& exchange, const std::string& routing_key, const std::string& message);

private:
    AMQP::TcpConnection* connection_;
    AMQP::TcpChannel* channel_;
};

