#include "rabbitmq.h"

RabbitMQ::RabbitMQ(const std::string& host, int port, const std::string& username,
                   const std::string& password, const std::string& vhost) {
    AMQP::Address address(host, port, AMQP::Login(username, password), vhost);
    connection_ = new AMQP::TcpConnection(new AMQP::LibEventHandler(), address);
    channel_ = new AMQP::TcpChannel(connection_);
}

RabbitMQ::~RabbitMQ() {
    delete channel_;
    delete connection_;
}

bool RabbitMQ::publish(const std::string& exchange, const std::string& routing_key,
                       const std::string& message) {
    try {
        channel_->publish(exchange, routing_key, message);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}
