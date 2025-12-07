#include <iostream>
#include <cstdlib>
#include "database.h"
#include "metrics.h"


int main(int argc, char* argv[]) {
    std::cout << "Metrics service starting..." << std::endl;

    DatabaseConfig db_config = load_database_config();
    
    std::cout << "Testing database connection..." << std::endl;
    if (!test_database_connection(db_config)) {
        std::cerr << "Failed to connect to database. Exiting." << std::endl;
        return 1;
    }
    std::cout << "Database connection successful." << std::endl;

    const char* port_env = std::getenv("GRPC_PORT");
    std::string port = port_env ? std::string(port_env) : "50051";
    std::string server_address = "0.0.0.0:" + port;

    std::cout << "Starting gRPC server on " << server_address << std::endl;
    
    run_grpc_server(server_address, db_config);

    return 0;
}
