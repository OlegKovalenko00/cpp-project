#include "database.h"

#include <cstdlib>
#include <iostream>
#include <pqxx/pqxx>

namespace {
    std::string build_connection_string(const DatabaseConfig& config) {
        return "host=" + config.host +
               " dbname=" + config.dbname +
               " user=" + config.user +
               " password=" + config.password;
    }
}

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
        pqxx::connection conn(build_connection_string(config));
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

bool save_page_view(const DatabaseConfig& config, const PageView& event) {
    try {
        pqxx::connection conn(build_connection_string(config));
        pqxx::work tx(conn);

        tx.exec_params(
            "INSERT INTO page_views (page, user_id, session_id, referrer) VALUES ($1, $2, $3, $4)",
            event.page,
            event.user_id.value_or(""),
            event.session_id.value_or(""),
            event.referrer.value_or("")
        );

        tx.commit();
        std::cout << "Saved page_view for page: " << event.page << std::endl;
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to save page_view: " << ex.what() << std::endl;
        return false;
    }
}

bool save_click_event(const DatabaseConfig& config, const ClickEvent& event) {
    try {
        pqxx::connection conn(build_connection_string(config));
        pqxx::work tx(conn);

        tx.exec_params(
            "INSERT INTO click_events (page, element_id, action, user_id, session_id) VALUES ($1, $2, $3, $4, $5)",
            event.page,
            event.element_id.value_or(""),
            event.action.value_or(""),
            event.user_id.value_or(""),
            event.session_id.value_or("")
        );

        tx.commit();
        std::cout << "Saved click_event for page: " << event.page << std::endl;
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to save click_event: " << ex.what() << std::endl;
        return false;
    }
}

bool save_performance_event(const DatabaseConfig& config, const PerformanceEvent& event) {
    try {
        pqxx::connection conn(build_connection_string(config));
        pqxx::work tx(conn);

        tx.exec_params(
            "INSERT INTO performance_events (page, ttfb_ms, fcp_ms, lcp_ms, total_page_load_ms, user_id, session_id) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7)",
            event.page,
            event.ttfb_ms.value_or(0.0),
            event.fcp_ms.value_or(0.0),
            event.lcp_ms.value_or(0.0),
            event.total_page_load_ms.value_or(0.0),
            event.user_id.value_or(""),
            event.session_id.value_or("")
        );

        tx.commit();
        std::cout << "Saved performance_event for page: " << event.page << std::endl;
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to save performance_event: " << ex.what() << std::endl;
        return false;
    }
}

bool save_error_event(const DatabaseConfig& config, const ErrorEvent& event) {
    try {
        pqxx::connection conn(build_connection_string(config));
        pqxx::work tx(conn);

        tx.exec_params(
            "INSERT INTO error_events (page, error_type, message, stack, severity, user_id, session_id) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7)",
            event.page,
            event.error_type.value_or(""),
            event.message.value_or(""),
            event.stack.value_or(""),
            event.severity.value_or(0),
            event.user_id.value_or(""),
            event.session_id.value_or("")
        );

        tx.commit();
        std::cout << "Saved error_event for page: " << event.page << std::endl;
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to save error_event: " << ex.what() << std::endl;
        return false;
    }
}

bool save_custom_event(const DatabaseConfig& config, const CustomEvent& event) {
    try {
        pqxx::connection conn(build_connection_string(config));
        pqxx::work tx(conn);

        tx.exec_params(
            "INSERT INTO custom_events (name, page, user_id, session_id) VALUES ($1, $2, $3, $4)",
            event.name,
            event.page.value_or(""),
            event.user_id.value_or(""),
            event.session_id.value_or("")
        );

        tx.commit();
        std::cout << "Saved custom_event: " << event.name << std::endl;
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to save custom_event: " << ex.what() << std::endl;
        return false;
    }
}
