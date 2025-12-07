#include "aggregator.h"
#include "database.h"

#include <iostream>

namespace aggregation {

Aggregator::Aggregator(Database& db) : database_(db) {
}

Aggregator::~Aggregator() = default;

void Aggregator::run() {
    std::string sql =
        "INSERT INTO aggregated_events ("
        "    time_bucket, project_id, page, event_type,"
        "    events_count, unique_users, unique_sessions,"
        "    avg_perf_ms, p95_perf_ms, errors_count"
        ") VALUES ("
        "    NOW(),"
        "    'demo-project',"
        "    '/test-page',"
        "    'page_view',"
        "    100,"
        "    10,"
        "    20,"
        "    123.0,"
        "    250.0,"
        "    3"
        ");";

    bool success = database_.executeQuery(sql);
    if (success) {
        std::cout << "Aggregation run completed successfully." << std::endl;
    } else {
        std::cerr << "Aggregation run failed." << std::endl;
    }
}

double Aggregator::calculateAverage(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& val : values) {
        sum += val;
    }
    return sum / static_cast<double>(values.size());
}

double Aggregator::calculateMin(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double min = values[0];
    for (const auto& val : values) {
        if (val < min) min = val;
    }
    return min;
}

double Aggregator::calculateMax(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double max = values[0];
    for (const auto& val : values) {
        if (val > max) max = val;
    }
    return max;
}

} // namespace aggregation

