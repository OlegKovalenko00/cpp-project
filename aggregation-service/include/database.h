#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

namespace aggregation {

class Database {
public:
    Database();
    ~Database();

    bool connect(const std::string& connectionString);
    void disconnect();

    // TODO: Методы для работы с метриками
    // std::vector<Metric> fetchMetrics();
    // void saveAggregatedMetrics(const AggregatedMetric& metric);
};

} // namespace aggregation

#endif // DATABASE_H

