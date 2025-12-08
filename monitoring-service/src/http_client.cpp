#include "http_client.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

namespace {
httplib::Client make_client(const std::string& host, int port) {
    httplib::Client cli(host, port);
    cli.set_connection_timeout(5);
    cli.set_read_timeout(5);
    return cli;
}
} 

PingResult check_ping(const std::string& host, int port) {
    PingResult result{false, 0, ""};
    try {
        auto cli = make_client(host, port);
        auto res = cli.Get("/health/ping");
        if (!res) {
            result.message = "unreachable";
            return result;
        }
        result.reachable = true;
        result.status_code = res->status;
        result.message = res->status == 200 ? "OK" : "Bad status";
        return result;
    } catch (const std::exception& e) {
        result.message = e.what();
        return result;
    }
}

ReadyResult check_ready(const std::string& host, int port) {
    ReadyResult result{false, 0, false, "", ""};
    try {
        auto cli = make_client(host, port);
        auto res = cli.Get("/health/ready");
        if (!res) {
            result.message = "unreachable";
            return result;
        }
        result.reachable = true;
        result.status_code = res->status;

        if (res->status == 200 || res->status == 503) {
            try {
                auto json = nlohmann::json::parse(res->body);
                result.db_connected = json.value("database_connected", false);
                result.service_status = json.value("status", std::string{});
            } catch (const std::exception& e) {
                result.message = std::string("parse error: ") + e.what();
            }
        } else {
            result.message = "unexpected status";
        }
        return result;
    } catch (const std::exception& e) {
        result.message = e.what();
        return result;
    }
}
