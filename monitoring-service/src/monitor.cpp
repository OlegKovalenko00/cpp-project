#include "monitor.h"
#include "http_client.h"
#include "logging.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

struct Service {
    std::string name;
    std::string host;
    int port;
    std::chrono::steady_clock::time_point last_ping;
    std::chrono::steady_clock::time_point last_ready;
};

void run_monitoring_loop() {
    const auto ping_interval = std::chrono::seconds(15);
    const auto ready_interval = std::chrono::seconds(45);
    auto now = std::chrono::steady_clock::now();

    std::vector<Service> services = {
        {"metrics-service", "metrics-service", 8080, now - ping_interval, now - ready_interval},
        {"aggregation-service", "aggregation-service", 8080, now - ping_interval, now - ready_interval}
    };

    while (true) {
        now = std::chrono::steady_clock::now();

        for (auto& service : services) {
            if (now - service.last_ping >= ping_interval) {
                auto ping = check_ping(service.host, service.port);
                service.last_ping = now;

                if (!ping.reachable) {
                    log_error(service.name + " unreachable (liveness failed)");
                } else if (ping.status_code == 200) {
                    log_info(service.name + " is alive");
                    log_debug(service.name + " heartbeat ok");
                } else {
                    log_error(service.name + " liveness failed, status=" + std::to_string(ping.status_code));
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
                    log_warning(service.name + " readiness unexpected status=" + std::to_string(ready.status_code));
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
