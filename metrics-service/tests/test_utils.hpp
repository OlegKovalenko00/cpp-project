#pragma once

#include "database.h"

#include <pqxx/pqxx>

#include <cstdlib>
#include <string>

inline DatabaseConfig TestDatabaseConfig() {
    DatabaseConfig config;
    const char* host = std::getenv("POSTGRES_HOST");
    const char* db = std::getenv("POSTGRES_DB");
    const char* user = std::getenv("POSTGRES_USER");
    const char* password = std::getenv("POSTGRES_PASSWORD");

    config.host = host ? host : "localhost";
    config.dbname = db ? db : "metrics_db";
    config.user = user ? user : "metrics_user";
    config.password = password ? password : "metrics_password";
    return config;
}

inline std::string BuildConnectionString(const DatabaseConfig& config) {
    return "host=" + config.host +
           " dbname=" + config.dbname +
           " user=" + config.user +
           " password=" + config.password;
}

inline void ResetDatabase(const DatabaseConfig& config) {
    pqxx::connection conn(BuildConnectionString(config));
    pqxx::work tx(conn);
    tx.exec("TRUNCATE TABLE page_views, click_events, performance_events, error_events, custom_events RESTART IDENTITY");
    tx.commit();
}

inline pqxx::result ExecuteQuery(const DatabaseConfig& config, const std::string& query) {
    pqxx::connection conn(BuildConnectionString(config));
    pqxx::work tx(conn);
    pqxx::result res = tx.exec(query);
    tx.commit();
    return res;
}
