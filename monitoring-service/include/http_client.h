#pragma once
#include <string>

struct PingResult {
    bool reachable;
    int status_code;
    std::string message;
};

struct ReadyResult {
    bool reachable;
    int status_code;
    bool db_connected;
    std::string service_status;
    std::string message;
};

PingResult check_ping(const std::string& host, int port);
ReadyResult check_ready(const std::string& host, int port);
