#include "monitoring_client.hpp"

#include <iostream>

MonitoringClient::MonitoringClient(std::string base_url) : base_url_(std::move(base_url)) {}

bool MonitoringClient::fetch(const std::string& path_with_query,
                             UptimeResponse* resp,
                             std::chrono::milliseconds timeout) {
    if (!resp) {
        return false;
    }

    httplib::Client cli(base_url_);
    cli.set_connection_timeout(timeout);
    cli.set_read_timeout(timeout);

    auto result = cli.Get(path_with_query.c_str());
    if (!result) {
        std::cerr << "[MonitoringClient] Request failed: "
                  << httplib::to_string(result.error()) << std::endl;
        return false;
    }

    if (result->status != 200) {
        std::cerr << "[MonitoringClient] Unexpected status " << result->status
                  << " for " << path_with_query << std::endl;
        return false;
    }

    try {
        auto body = nlohmann::json::parse(result->body);
        *resp = body.get<UptimeResponse>();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[MonitoringClient] Failed to parse response: " << e.what() << std::endl;
        return false;
    }
}

bool MonitoringClient::GetUptime(const std::string& service,
                                 const std::optional<std::string>& period,
                                 UptimeResponse* resp,
                                 std::chrono::milliseconds timeout) {
    std::string path = "/uptime?service=" + httplib::detail::encode_url(service);
    if (period.has_value()) {
        path += "&period=" + httplib::detail::encode_url(period.value());
    }
    return fetch(path, resp, timeout);
}

bool MonitoringClient::GetUptimeDay(const std::string& service,
                                    UptimeResponse* resp,
                                    std::chrono::milliseconds timeout) {
    std::string path = "/uptime/day?service=" + httplib::detail::encode_url(service);
    return fetch(path, resp, timeout);
}

bool MonitoringClient::GetUptimeWeek(const std::string& service,
                                     UptimeResponse* resp,
                                     std::chrono::milliseconds timeout) {
    std::string path = "/uptime/week?service=" + httplib::detail::encode_url(service);
    return fetch(path, resp, timeout);
}

bool MonitoringClient::GetUptimeMonth(const std::string& service,
                                      UptimeResponse* resp,
                                      std::chrono::milliseconds timeout) {
    std::string path = "/uptime/month?service=" + httplib::detail::encode_url(service);
    return fetch(path, resp, timeout);
}

bool MonitoringClient::GetUptimeYear(const std::string& service,
                                     UptimeResponse* resp,
                                     std::chrono::milliseconds timeout) {
    std::string path = "/uptime/year?service=" + httplib::detail::encode_url(service);
    return fetch(path, resp, timeout);
}
