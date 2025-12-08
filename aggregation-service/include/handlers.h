#ifndef HANDLERS_H
#define HANDLERS_H

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>

namespace aggregation {

class Database;

// ==================== HTTP Handlers ====================

class HttpHandlers {
public:
    explicit HttpHandlers(Database& db);

    // GET /health/ping — проверка доступности сервиса
    void handleHealthPing(const httplib::Request& req, httplib::Response& res);

    // GET /health/ready — проверка готовности (включая БД)
    void handleHealthReady(const httplib::Request& req, httplib::Response& res);

    // Регистрация маршрутов
    void registerRoutes(httplib::Server& server);

private:
    Database& database_;
};

// ==================== Response Structures ====================

struct HealthResponse {
    std::string status;
    std::string service;
    std::string timestamp;
};

struct ReadyResponse {
    std::string status;
    bool database_connected;
    std::string timestamp;
};

// JSON сериализация
inline void to_json(nlohmann::json& j, const HealthResponse& r) {
    j = nlohmann::json{
        {"status", r.status},
        {"service", r.service},
        {"timestamp", r.timestamp}
    };
}

inline void to_json(nlohmann::json& j, const ReadyResponse& r) {
    j = nlohmann::json{
        {"status", r.status},
        {"database_connected", r.database_connected},
        {"timestamp", r.timestamp}
    };
}

} // namespace aggregation

#endif // HANDLERS_H

