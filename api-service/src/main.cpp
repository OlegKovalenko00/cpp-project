#include "handlers.hpp"

#include <httplib.h>
#include <iostream>

int main() {
    httplib::Server server;

    registerRoutes(server);

    std::cout << "Server starting on http://localhost:8080" << std::endl;

    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server on port 8080" << std::endl;
        return 1;
    }

    return 0;
}
