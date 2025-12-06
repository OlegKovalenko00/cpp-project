#include "database.h"

#include <cstdlib>
#include <iostream>
#include <pqxx/pqxx>

DatabaseConfig load_database_config() {
    const char* host_env = std::getenv("POSTGRES_HOST");
    const char* db_env = std::getenv("POSTGRES_DB");
    const char* user_env = std::getenv("POSTGRES_USER");
    const char* password_env = std::getenv("POSTGRES_PASSWORD");

    DatabaseConfig config;
    config.host = host_env ? std::string(host_env) : std::string("localhost");
    config.dbname = db_env ? std::string(db_env) : std::string("metrics_db");
    config.user = user_env ? std::string(user_env) : std::string("metrics_user");
    config.password = password_env ? std::string(password_env) : std::string("metrics_password");

    return config;
}

bool test_database_connection(const DatabaseConfig& config) {
    try {
        std::string conn_str =
            "host=" + config.host +
            " dbname=" + config.dbname +
            " user=" + config.user +
            " password=" + config.password;

        pqxx::connection conn(conn_str);
        if (!conn.is_open()) {
            std::cerr << "Database connection is not open" << std::endl;
            return false;
        }

        pqxx::work tx(conn);
        pqxx::result r = tx.exec("SELECT 1");
        tx.commit();

        if (r.empty()) {
            std::cerr << "Database SELECT 1 returned empty result" << std::endl;
            return false;
        }

        std::cout << "Database connection successful" << std::endl;
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Database connection failed: " << ex.what() << std::endl;
        return false;
    }
}
