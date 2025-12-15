#include "handlers.hpp"
#include "rabbitmq.hpp"

#include <httplib.h>
#include <iostream>

int main() {
    httplib::Server server;

    // Запуск асинхронного publisher RabbitMQ
    getRabbitMQ().start_async_publisher();

    registerRoutes(server);

    std::cout << "Server starting on http://localhost:8080" << std::endl;

    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server on port 8080" << std::endl;
        getRabbitMQ().stop_async_publisher();
        return 1;
    }

    getRabbitMQ().stop_async_publisher();
    return 0;
}
