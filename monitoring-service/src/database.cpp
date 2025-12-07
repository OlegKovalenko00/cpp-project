#include "database.h"
#include <pqxx/pqxx>
#include <iostream>

static pqxx::connection* conn = nullptr;

void db_init() {
    try {
        conn = new pqxx::connection(
            "dbname=postgres user=postgres password=postgres host=localhost"
        );
        std::cout << "Connected to PostgreSQL\n";
    } catch (const std::exception& e) {
        std::cerr << "DB connection error: " << e.what() << std::endl;
    }
}

void db_write_result(const std::string& service_name, const std::string& url, bool ok) {
    if (!conn || !conn->is_open()) {
        std::cerr << "DB not connected!\n";
        return;
    }

    try {
        pqxx::work txn(*conn);
        pqxx::params params;
        params.append(service_name);
        params.append(ok ? "OK" : "FAIL");
        txn.exec(
            "INSERT INTO logs(service_name, log_message, timestamp) VALUES ($1, $2, NOW())",
            params
        );
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "DB write error: " << e.what() << std::endl;
    }
}