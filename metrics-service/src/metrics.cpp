#include "metrics.h"

Metric make_metric_from_unix(const std::string& name, double value, std::int64_t unix_second) {
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::time_point{std::chrono::seconds(unix_second)};

    Metric metric;
    metric.name = name;
    metric.value = value;
    metric.timestamp = tp;
    return metric;
}