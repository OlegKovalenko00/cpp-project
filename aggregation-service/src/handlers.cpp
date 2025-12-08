#include "handlers.h"
#include "database.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace aggregation {

using json = nlohmann::json;

// Вспомогательная функция для получения текущего времени в ISO 8601
static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now{};
    gmtime_r(&time_t_now, &tm_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

HttpHandlers::HttpHandlers(Database& db) : database_(db) {
}

// GET /health/ping
void HttpHandlers::handleHealthPing(const httplib::Request& req, httplib::Response& res) {
    HealthResponse response{
        "ok",
        "aggregation-service",
        getCurrentTimestamp()
    };

    res.status = 200;
    res.set_content(json(response).dump(), "application/json");

    std::cout << "[HTTP] GET /health/ping -> 200 OK" << std::endl;
}

// GET /health/ready
void HttpHandlers::handleHealthReady(const httplib::Request& req, httplib::Response& res) {
    bool dbConnected = database_.isConnected();

    ReadyResponse response{
        dbConnected ? "ready" : "not_ready",
        dbConnected,
        getCurrentTimestamp()
    };

    res.status = dbConnected ? 200 : 503;
    res.set_content(json(response).dump(), "application/json");

    std::cout << "[HTTP] GET /health/ready -> " << res.status
              << (dbConnected ? " (DB connected)" : " (DB disconnected)") << std::endl;
}

// Регистрация маршрутов
void HttpHandlers::registerRoutes(httplib::Server& server) {
    server.Get("/health/ping", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealthPing(req, res);
    });

    server.Get("/health/ready", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealthReady(req, res);
    });

    std::cout << "[HTTP] Routes registered:" << std::endl;
    std::cout << "  GET /health/ping  - liveness probe" << std::endl;
    std::cout << "  GET /health/ready - readiness probe" << std::endl;
}

} // namespace aggregation

