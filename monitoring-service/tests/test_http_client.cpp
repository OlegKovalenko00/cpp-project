#include <chrono>
#include <thread>
#include <httplib.h>
#include <cassert>

#include "http_client.h"

int main() {
    const int port = 9095;
    httplib::Server server;

    server.Get("/health/ping", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    server.Get("/health/ready", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        res.set_content(R"({"status":"ready","database_connected":true})", "application/json");
    });

    std::thread server_thread([&]() { server.listen("127.0.0.1", port); });
    for (int i = 0; i < 50 && !server.is_running(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto ping = check_ping("127.0.0.1", port);
    assert(ping.reachable);
    assert(ping.status_code == 200);
    assert(ping.message == "OK");

    auto ready = check_ready("127.0.0.1", port);
    assert(ready.reachable);
    assert(ready.status_code == 200);
    assert(ready.db_connected == true);
    assert(ready.service_status == "ready");

    server.stop();
    if (server_thread.joinable()) server_thread.join();

    auto bad_ping = check_ping("127.0.0.1", 6550);
    assert(!bad_ping.reachable);

    return 0;
}
