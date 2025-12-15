#pragma once
#include <string>
#include <optional>

struct PeriodStat {
    long long ok;
    long long total;
};

struct UptimeStats {
    PeriodStat day;
    PeriodStat week;
    PeriodStat month;
    PeriodStat year;
};

void db_init();
void db_write_result(const std::string& service_name, const std::string& url, bool ok);
bool db_is_ready();
std::optional<UptimeStats> db_get_uptime_stats(const std::string& service_name, std::string& error);
