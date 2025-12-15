#include "database.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <pqxx/pqxx>
#include <cstdlib>

namespace {
std::mutex db_mutex;
std::unique_ptr<pqxx::connection> conn;

std::string get_env(const char* name, const std::string& default_value) {
    const char* val = std::getenv(name);
    return val ? val : default_value;
}

int get_env_int(const char* name, int default_value) {
    const char* val = std::getenv(name);
    return val ? std::stoi(val) : default_value;
}
}

void db_init() {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (conn && conn->is_open()) {
        return;
    }

    try {
        const auto host = get_env("POSTGRES_HOST", "localhost");
        const auto db = get_env("POSTGRES_DB", "postgres");
        const auto user = get_env("POSTGRES_USER", "postgres");
        const auto password = get_env("POSTGRES_PASSWORD", "postgres");
        const auto port = get_env_int("POSTGRES_PORT", 5432);

        const auto conninfo = "host=" + host + " port=" + std::to_string(port) + " dbname=" + db +
                              " user=" + user + " password=" + password;

        conn = std::make_unique<pqxx::connection>(conninfo);
        std::cout << "Connected to PostgreSQL\n";
    } catch (const std::exception& e) {
        std::cerr << "DB connection error: " << e.what() << std::endl;
    }
}

bool db_is_ready() {
    std::lock_guard<std::mutex> lock(db_mutex);
    return conn && conn->is_open();
}

void db_write_result(const std::string& service_name, const std::string& url, bool ok) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) {
        std::cerr << "DB not connected!\n";
        return;
    }

    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO logs(service_name, log_message, timestamp) VALUES ($1, $2, NOW())",
            service_name, ok ? "OK" : "FAIL");
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "DB write error: " << e.what() << std::endl;
    }
}

std::optional<UptimeStats> db_get_uptime_stats(const std::string& service_name, std::string& error) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) {
        error = "DB not connected";
        return std::nullopt;
    }

    try {
        pqxx::work txn(*conn);
        auto res = txn.exec_params(
            R"(SELECT
                    COUNT(*) FILTER (WHERE timestamp >= NOW() - INTERVAL '1 day' AND log_message = 'OK')   AS day_ok,
                    COUNT(*) FILTER (WHERE timestamp >= NOW() - INTERVAL '1 day')                         AS day_total,
                    COUNT(*) FILTER (WHERE timestamp >= NOW() - INTERVAL '1 week' AND log_message = 'OK') AS week_ok,
                    COUNT(*) FILTER (WHERE timestamp >= NOW() - INTERVAL '1 week')                        AS week_total,
                    COUNT(*) FILTER (WHERE timestamp >= NOW() - INTERVAL '1 month' AND log_message = 'OK') AS month_ok,
                    COUNT(*) FILTER (WHERE timestamp >= NOW() - INTERVAL '1 month')                        AS month_total,
                    COUNT(*) FILTER (WHERE timestamp >= NOW() - INTERVAL '1 year' AND log_message = 'OK')  AS year_ok,
                    COUNT(*) FILTER (WHERE timestamp >= NOW() - INTERVAL '1 year')                         AS year_total
                FROM logs
                WHERE service_name = $1)",
            service_name);

        if (res.empty()) {
            return UptimeStats{0, 0, 0, 0};
        }

        const auto& row = res[0];
        UptimeStats stats{
            {row["day_ok"].as<long long>(0), row["day_total"].as<long long>(0)},
            {row["week_ok"].as<long long>(0), row["week_total"].as<long long>(0)},
            {row["month_ok"].as<long long>(0), row["month_total"].as<long long>(0)},
            {row["year_ok"].as<long long>(0), row["year_total"].as<long long>(0)},
        };
        return stats;
    } catch (const std::exception& e) {
        error = e.what();
        return std::nullopt;
    }
}
