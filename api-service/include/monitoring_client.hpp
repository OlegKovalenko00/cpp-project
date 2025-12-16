#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <chrono>

#include "models.hpp"

class MonitoringClient {
  public:
    explicit MonitoringClient(std::string base_url);

    bool GetUptime(const std::string& service,
                   const std::optional<std::string>& period,
                   UptimeResponse* resp,
                   std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

    bool GetUptimeDay(const std::string& service,
                      UptimeResponse* resp,
                      std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
    bool GetUptimeWeek(const std::string& service,
                       UptimeResponse* resp,
                       std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
    bool GetUptimeMonth(const std::string& service,
                        UptimeResponse* resp,
                        std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
    bool GetUptimeYear(const std::string& service,
                       UptimeResponse* resp,
                       std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

  private:
    bool fetch(const std::string& path_with_query,
               UptimeResponse* resp,
               std::chrono::milliseconds timeout);

    std::string base_url_;
};
