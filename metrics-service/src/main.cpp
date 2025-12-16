#include <cstdlib>
#include <exception>
#include <iostream>

#include "database.h"
#include "http_handler.h"
#include "message_processor.h"
#include "metrics.h"
#include "rabbitmq.h"

int main(int argc, char* argv[]) {
    std::cout << "Metrics service starting..." << std::endl;

    DatabaseConfig db_config = load_database_config();
    std::cout << "Testing database connection..." << std::endl;
    if (!test_database_connection(db_config)) {
        std::cerr << "Failed to connect to database. Exiting." << std::endl;
        return 1;
    }
    std::cout << "Database connection successful." << std::endl;

    const char* http_port_env = std::getenv("HTTP_PORT");
    int http_port = http_port_env ? std::stoi(http_port_env) : 8080;
    HttpHandler http_handler(http_port, [&db_config]() {
        return test_database_connection(db_config);
    });
    http_handler.start();

    RabbitMQConfig rabbit_config = load_rabbitmq_config();
    RabbitMQConsumer rabbit(rabbit_config);
    std::cout << "Connecting to RabbitMQ at " << rabbit_config.host << ":" << rabbit_config.port << std::endl;
    try {
        rabbit.connect();
        rabbit.subscribe([&db_config](const std::string& queue, const std::string& message) {
            process_message(queue, message, db_config);
        });
        rabbit.start();
    } catch (const std::exception& ex) {
        std::cerr << "RabbitMQ initialization failed: " << ex.what() << std::endl;
        http_handler.stop();
        return 1;
    }

    const char* port_env = std::getenv("GRPC_PORT");
    std::string port = port_env ? std::string(port_env) : "50051";
    std::string server_address = "0.0.0.0:" + port;
    std::cout << "Starting gRPC server on " << server_address << std::endl;
    run_grpc_server(server_address, db_config);

    rabbit.stop();
    http_handler.stop();
    return 0;
}
