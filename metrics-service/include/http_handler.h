#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>

class HttpHandler {
public:
    using DatabaseCheckFunc = std::function<bool()>;

    HttpHandler(int port, DatabaseCheckFunc db_check);
    ~HttpHandler();

    void start();
    void stop();

private:
    int port_;
    DatabaseCheckFunc db_check_;
    std::atomic<bool> running_{false};
    std::thread server_thread_;
    
    class Impl;
    std::unique_ptr<Impl> impl_;
    
    std::string getCurrentTimestamp();
};
