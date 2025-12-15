#include <iostream>
#include <thread>

#include "database.h"
#include "http_server.h"
#include "monitor.h"

int main() {
    std::cout << "Monitoring service started" << std::endl;

    db_init();

    std::thread monitoring_thread(run_monitoring_loop);
    run_http_server();

    monitoring_thread.join();
    return 0;
}
