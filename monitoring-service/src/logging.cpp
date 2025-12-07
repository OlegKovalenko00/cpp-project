#include "logging.h"

#include <iostream>

void log_info(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void log_error(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}
