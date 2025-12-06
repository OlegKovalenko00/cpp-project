#include <iostream>
#include "database.h"
int main() {
    std::cout << "Metrics service started" << std::endl;

    DatabaseConfig config = load_database_config();
    bool status = test_database_connection(config);
    
    if (status) {
        std::cout << "Database test query succeeded" << std::endl;
    } else {
        std::cout << "Database test query failed" << std::endl;
    }
}
