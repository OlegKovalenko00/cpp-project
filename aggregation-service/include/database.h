#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <libpq-fe.h>

namespace aggregation {

class Database {
public:
    Database();
    ~Database();

    bool connect(const std::string& connectionString);
    void disconnect();

	bool isConnected() const;

    bool initializeSchema();

    bool executeQuery(const std::string& query);

    // TODO: Методы для работы с метриками
    // std::vector<Metric> fetchMetrics();
    // void saveAggregatedMetrics(const AggregatedMetric& metric);

private:
    PGconn* dbConnection_;
};

} // namespace aggregation

#endif // DATABASE_H

