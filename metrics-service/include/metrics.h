#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

struct Metric {
    std::string name;
    double value;
    std::chrono::system_clock::time_point timestamp;
};

Metric make_metric_from_unix(const std::string& name, double value, std::int64_t unix_second);