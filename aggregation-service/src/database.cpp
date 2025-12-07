#include "database.h"

#include <iostream>

namespace aggregation {

Database::Database() : dbConnection_(nullptr) {}

Database::~Database() {
    disconnect();
}

bool Database::connect(const std::string& connectionString) {
    disconnect();

    dbConnection_ = PQconnectdb(connectionString.c_str());

    if (dbConnection_ == nullptr) {
        std::cerr << "Failed to allocate PGconn" << std::endl;
        return false;
    }

    if (PQstatus(dbConnection_) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: "
                  << PQerrorMessage(dbConnection_) << std::endl;
        disconnect();
        return false;
    }

    std::cout << "Connected to database successfully" << std::endl;
    return true;
}

void Database::disconnect() {
    if (dbConnection_ != nullptr) {
        PQfinish(dbConnection_);
        dbConnection_ = nullptr;
    }
}

bool Database::isConnected() const {
    return dbConnection_ != nullptr && PQstatus(dbConnection_) == CONNECTION_OK;
}

bool Database::initializeSchema() {
    if (!isConnected()) {
        std::cerr << "Cannot initialize schema: no active database connection" << std::endl;
        return false;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS aggregated_events ("
        "    id SERIAL PRIMARY KEY,"
        "    time_bucket TIMESTAMPTZ NOT NULL,"
        "    project_id TEXT NOT NULL,"
        "    page TEXT,"
        "    event_type TEXT NOT NULL,"
        "    events_count BIGINT NOT NULL DEFAULT 0,"
        "    unique_users BIGINT,"
        "    unique_sessions BIGINT,"
        "    avg_perf_ms DOUBLE PRECISION,"
        "    p95_perf_ms DOUBLE PRECISION,"
        "    errors_count BIGINT,"
        "    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()"
        ");"
        "CREATE TABLE IF NOT EXISTS aggregation_watermark ("
        "    id INTEGER PRIMARY KEY,"
        "    last_aggregated_at TIMESTAMPTZ NOT NULL"
        ");"
        "INSERT INTO aggregation_watermark (id, last_aggregated_at)"
        "VALUES (1, '1970-01-01T00:00:00Z')"
        "ON CONFLICT (id) DO NOTHING;";

    PGresult* res = PQexec(dbConnection_, sql);

    if (res == nullptr) {
        std::cerr << "Failed to execute initializationSchema: null result" << std::endl;
        return false;
    }

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Failed to initialize schema: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    std::cout << "Database schema initialized successfully" << std::endl;
    return true;
}

bool Database::executeQuery(const std::string& query) {
    if (!isConnected()) {
        std::cerr << "Cannot execute query: no active database connection" << std::endl;
        return false;
    }

    PGresult* res = PQexec(dbConnection_, query.c_str());

    if (res == nullptr) {
        std::cerr << "Failed to execute query: null result" << std::endl;
        return false;
    }

    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        std::cerr << "Query execution failed: " << PQerrorMessage(dbConnection_) << std::endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

} // namespace aggregation
