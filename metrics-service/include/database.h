#pragma once

#include <string>

struct DatabaseConfig {
    std::string host;
    std::string dbname;
    std::string user;
    std::string password;
};

DatabaseConfig load_database_config();
bool test_database_connection(const DatabaseConfig& config);
