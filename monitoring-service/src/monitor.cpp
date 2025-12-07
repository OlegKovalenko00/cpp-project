#include "monitor.h"
#include "http_client.h"
#include "database.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

struct Service {
    std::string name;
    std::string url;
};

void run_monitoring_loop() {
    db_init();

    std::vector<Service> services = {
        {"service1", "http://localhost:8080/health"},
        {"service2", "http://localhost:8081/health"},
        {"service3", "http://localhost:8082/health"}
    };

    while (true) {
        std::cout << "\nChecking services..." << std::endl;

        for (auto& s : services) {
            std::string msg;
            bool ok = check_health(s.url, msg);
            db_write_result(s.name, s.url, ok);

            std::cout << s.name << " -> " 
                      << (ok ? "OK" : "FAIL")
                      << " (" << msg << ")"
                      << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}