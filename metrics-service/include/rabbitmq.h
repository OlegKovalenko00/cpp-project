#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

struct RabbitMQConfig {
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string vhost;
    std::vector<std::string> queues;
};

RabbitMQConfig load_rabbitmq_config();


// Thread-safe queue for inter-thread message passing
struct RabbitMQMessage {
    std::string queue;
    std::string body;
};

class MessageQueue {
public:
    void push(const RabbitMQMessage& msg) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(msg);
        cv_.notify_one();
    }
    bool pop(RabbitMQMessage& msg) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return !queue_.empty() || stop_; });
        if (queue_.empty()) return false;
        msg = queue_.front();
        queue_.pop();
        return true;
    }
    void stop() {
        std::lock_guard<std::mutex> lock(mtx_);
        stop_ = true;
        cv_.notify_all();
    }
private:
    std::queue<RabbitMQMessage> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stop_ = false;
};

class RabbitMQConsumer {
public:
    RabbitMQConsumer(const RabbitMQConfig& config);
    ~RabbitMQConsumer();

    void connect();
    void subscribe(); // Кладёт сообщения в очередь
    void start();
    void stop();

    MessageQueue& get_message_queue() { return message_queue_; }

private:
    RabbitMQConfig config_;
    class Impl;
    std::unique_ptr<Impl> impl_;
    std::thread event_thread_;
    MessageQueue message_queue_;
    std::atomic<bool> running_{false};
};
