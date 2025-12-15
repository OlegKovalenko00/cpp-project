#include "http_server.h"

#include "database.h"
#include "logging.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
std::string get_env(const char* name, const std::string& default_value) {
    const char* val = std::getenv(name);
    return val ? val : default_value;
}

int get_env_int(const char* name, int default_value) {
    const char* val = std::getenv(name);
    return val ? std::stoi(val) : default_value;
}

std::string now_iso8601() {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&tt, &tm);

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buffer;
}

void write_json(httplib::Response& res, int status, const nlohmann::json& body) {
    res.status = status;
    res.set_content(body.dump(), "application/json");
}
}  

void run_http_server() {
    const int port = get_env_int("HTTP_PORT", 8083);
    httplib::Server server;

    server.Get("/health/ping", [](const httplib::Request&, httplib::Response& res) {
        nlohmann::json body = {
            {"status", "ok"},
            {"service", "monitoring-service"},
            {"timestamp", now_iso8601()}};
        write_json(res, 200, body);
    });

    server.Get("/health/ready", [](const httplib::Request&, httplib::Response& res) {
        const bool ready = db_is_ready();
        nlohmann::json body = {{"status", ready ? "ready" : "not_ready"},
                               {"database_connected", ready},
                               {"timestamp", now_iso8601()}};
        write_json(res, ready ? 200 : 503, body);
    });

    auto respond_with_uptime = [](const httplib::Request& req, httplib::Response& res,
                                  const std::string& forced_period) {
        if (!req.has_param("service")) {
            write_json(res, 400, {{"error", "missing query param: service"}});
            return;
        }

        const auto service_name = req.get_param_value("service");
        const auto period = forced_period.empty() && req.has_param("period")
                                ? req.get_param_value("period")
                                : forced_period;

        std::string error;
        auto stats = db_get_uptime_stats(service_name, error);
        if (!stats.has_value()) {
            log_error("Failed to fetch uptime stats for " + service_name + ": " + error);
            write_json(res, 500, {{"error", "failed to read stats"}, {"details", error}});
            return;
        }

        auto build_period = [](const PeriodStat& p) {
            double percent = (p.total > 0) ? (static_cast<double>(p.ok) * 100.0 / p.total) : 0.0;
            return nlohmann::json{{"ok", p.ok}, {"total", p.total}, {"percent", percent}};
        };

        auto periods = nlohmann::json{
            {"day", build_period(stats->day)},
            {"week", build_period(stats->week)},
            {"month", build_period(stats->month)},
            {"year", build_period(stats->year)},
        };

        nlohmann::json body = {{"service", service_name},
                               {"period", period.empty() ? "all" : period},
                               {"periods", periods}};

        if (!period.empty()) {
            const auto it = periods.find(period);
            if (it == periods.end()) {
                write_json(res, 400, {{"error", "invalid period. expected day|week|month|year"}});
                return;
            }
            body["periods"] = {{period, *it}};
        }

        write_json(res, 200, body);
    };

    server.Get("/uptime", [respond_with_uptime](const httplib::Request& req, httplib::Response& res) {
        respond_with_uptime(req, res, "");
    });
    server.Get("/uptime/day",
               [respond_with_uptime](const httplib::Request& req, httplib::Response& res) {
                   respond_with_uptime(req, res, "day");
               });
    server.Get("/uptime/week",
               [respond_with_uptime](const httplib::Request& req, httplib::Response& res) {
                   respond_with_uptime(req, res, "week");
               });
    server.Get("/uptime/month",
               [respond_with_uptime](const httplib::Request& req, httplib::Response& res) {
                   respond_with_uptime(req, res, "month");
               });
    server.Get("/uptime/year",
               [respond_with_uptime](const httplib::Request& req, httplib::Response& res) {
                   respond_with_uptime(req, res, "year");
               });

    std::cout << "[HTTP] monitoring-service listening on 0.0.0.0:" << port << std::endl;
    std::cout << "[HTTP] Routes:" << std::endl;
    std::cout << "  GET /health/ping" << std::endl;
    std::cout << "  GET /health/ready" << std::endl;
    std::cout << "  GET /uptime?service=name[&period=day|week|month|year]" << std::endl;
    std::cout << "  GET /uptime/day|week|month|year?service=name" << std::endl;

    server.listen("0.0.0.0", port);
}
