#include "handlers.h"
#include <httplib.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

class HttpHandler::Impl {
public:
    httplib::Server server;
};

std::string HttpHandler::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now{};
    gmtime_r(&time_t_now, &tm_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

HttpHandler::HttpHandler(int port)
    : port_(port), impl_(std::make_unique<Impl>()) {
    
    impl_->server.Get("/health/ping", [this](const httplib::Request& req, httplib::Response& res) {
        std::string timestamp = getCurrentTimestamp();
        std::string response = R"({"status":"ok","service":"metrics-service","timestamp":")" + timestamp + R"("})";
        res.set_content(response, "application/json");
        std::cout << "[HTTP] GET /health/ping -> 200 OK" << std::endl;
    });


    impl_->server.Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
        std::string timestamp = getCurrentTimestamp();
        std::string response = R"({"status":"healthy","service":"metrics-service","timestamp":")" + timestamp + R"("})";
        res.set_content(response, "application/json");
    });

    impl_->server.Get("/ping", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("pong", "text/plain");
    });
}

HttpHandler::~HttpHandler() {
    stop();
}

void HttpHandler::start() {
    running_ = true;
    server_thread_ = std::thread([this]() {
        std::cout << "[HTTP] Server listening on port " << port_ << std::endl;
        std::cout << "[HTTP] Routes registered:" << std::endl;
        std::cout << "  GET /health/ping  - liveness probe" << std::endl;
        std::cout << "  GET /health/ready - readiness probe" << std::endl;
        std::cout << "  GET /health       - health check" << std::endl;
        std::cout << "  GET /ping         - simple ping" << std::endl;
        impl_->server.listen("0.0.0.0", port_);
    });
}

void HttpHandler::stop() {
    if (running_) {
        running_ = false;
        impl_->server.stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }
}
