#include "logging.h"

#include <iostream>

void log_debug(const std::string& message) {
    std::cout << "[DEBUG] " << message << std::endl;
}

void log_info(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void log_warning(const std::string& message) {
    std::cerr << "[WARNING] " << message << std::endl;
}

void log_error(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}
