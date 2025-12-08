#include "database.h"

#include <iostream>
#include <pqxx/pqxx>

static pqxx::connection* conn = nullptr;

void db_init() {
    try {
        conn =
            new pqxx::connection("dbname=postgres user=postgres password=postgres host=localhost");
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
        txn.exec_params(
            "INSERT INTO logs(service_name, log_message, timestamp) VALUES ($1, $2, NOW())",
            service_name, ok ? "OK" : "FAIL");
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "DB write error: " << e.what() << std::endl;
    }
}