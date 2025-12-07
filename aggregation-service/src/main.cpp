#include <iostream>
#include <cstdlib>
#include <string>

#include "aggregator.h"
#include <database.h>


std::string GetEnvVar(const std::string& varName, const std::string& defaultValue) {
    const char* value = std::getenv(varName.c_str());
    if (value == nullptr) {
        return defaultValue;
    }
    return std::string(value);
}

std::string BuildPostrgresConnectionString() {
    std::string host = GetEnvVar("AGG_DB_HOST", "localhost");
    std::string port = GetEnvVar("AGG_DB_PORT", "5434");
    std::string dbname = GetEnvVar("AGG_DB_NAME", "aggregation_db");
    std::string user = GetEnvVar("AGG_DB_USER", "agguser");
    std::string password = GetEnvVar("AGG_DB_PASSWORD", "aggpassword");

    std::string connectionString = "host=" + host +
                                   " port=" + port +
                                   " dbname=" + dbname +
                                   " user=" + user +
                                   " password=" + password;
    return connectionString;
}

int main() {
    std::cout << "Aggregation Service started" << std::endl;

    std::string connectionString = BuildPostrgresConnectionString();

    aggregation::Database database;
    if (!database.connect(connectionString)) {
        std::cerr << "Failed to connect to the database. Exiting." << std::endl;
        return 1;
    }

    if (!database.initializeSchema()) {
        std::cerr << "Failed to initialize database schema. Exiting." << std::endl;
        return 1;
    }

    aggregation::Aggregator aggregator(database);
    aggregator.run();

    return 0;
}
