#include "monitor.h"

#include "http_client.h"
#include "logging.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

struct Service {
    std::string name;
    std::string host;
    int port;
    std::chrono::steady_clock::time_point last_ping;
    std::chrono::steady_clock::time_point last_ready;
};

static std::string get_env(const char* name, const std::string& default_value) {
    const char* val = std::getenv(name);
    return val ? val : default_value;
}

static int get_env_int(const char* name, int default_value) {
    const char* val = std::getenv(name);
    return val ? std::stoi(val) : default_value;
}

void run_monitoring_loop() {
    const auto ping_interval = std::chrono::seconds(15);
    const auto ready_interval = std::chrono::seconds(45);
    auto now = std::chrono::steady_clock::now();

    std::vector<Service> services = {
        {"api-service", get_env("API_SERVICE_HOST", "api-service"),
         get_env_int("API_SERVICE_PORT", 8080), now - ping_interval, now - ready_interval},
        {"metrics-service", get_env("METRICS_SERVICE_HOST", "metrics-service"),
         get_env_int("METRICS_SERVICE_PORT", 8081), now - ping_interval, now - ready_interval},
        {"aggregation-service", get_env("AGGREGATION_SERVICE_HOST", "aggregation-service"),
         get_env_int("AGGREGATION_SERVICE_PORT", 8082), now - ping_interval, now - ready_interval}};

    log_info("Monitoring started. Targets:");
    for (const auto& s : services) {
        log_info("  " + s.name + " -> " + s.host + ":" + std::to_string(s.port));
    }

    while (true) {
        now = std::chrono::steady_clock::now();

        for (auto& service : services) {
            if (now - service.last_ping >= ping_interval) {
                auto ping = check_ping(service.host, service.port);
                service.last_ping = now;

                if (!ping.reachable) {
                    log_error(service.name + " unreachable (liveness failed)");
                } else if (ping.status_code == 200) {
                    log_info(service.name + " IS ALIVE");
                    log_debug(service.name + " HEARTBEAT OK");
                } else {
                    log_error(service.name +
                              " liveness failed, status=" + std::to_string(ping.status_code));
                }
            }

            if (now - service.last_ready >= ready_interval) {
                auto ready = check_ready(service.host, service.port);
                service.last_ready = now;

                if (!ready.reachable) {
                    log_warning(service.name + " readiness check failed");
                } else if (ready.status_code == 503) {
                    log_warning(service.name + " not ready");
                } else if (ready.status_code == 200) {
                    if (ready.db_connected) {
                        log_info(service.name + " fully operational");
                    } else {
                        log_warning(service.name + " dependency failure (DB disconnected)");
                    }
                } else {
                    log_warning(service.name + " readiness unexpected status=" +
                                std::to_string(ready.status_code));
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
