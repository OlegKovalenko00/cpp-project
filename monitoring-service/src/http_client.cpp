#include "http_client.h"
#include "httplib.h"
#include <iostream>

bool check_health(const std::string& url, std::string& msg) {
    try {
        auto protocol_end = url.find("://");
        auto host_start = protocol_end + 3;

        auto path_start = url.find('/', host_start);
        std::string host_port = url.substr(host_start, path_start - host_start);
        std::string path = url.substr(path_start);

        auto colon_pos = host_port.find(':');
        std::string host = host_port.substr(0, colon_pos);
        int port = std::stoi(host_port.substr(colon_pos + 1));

        httplib::Client cli(host, port);
        auto res = cli.Get(path.c_str());

        if (res && res->status == 200) {
            msg = "OK";
            return true;
        } else {
            msg = "Bad Response or non-200 status";
            return false;
        }
    }
    catch (const std::exception& e) {
        msg = e.what();
        return false;
    }
}