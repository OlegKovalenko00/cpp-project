#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <chrono>
#include <libpq-fe.h>

namespace aggregation {

// Forward declarations
struct AggregatedPageViews;
struct AggregatedClicks;
struct AggregatedPerformance;
struct AggregatedErrors;
struct AggregatedCustomEvents;
struct AggregationResult;

class Database {
public:
    Database();
    ~Database();

    bool connect(const std::string& connectionString);
    void disconnect();
    bool isConnected() const;

    bool initializeSchema();
    bool executeQuery(const std::string& query);

    // Watermark методы
    std::chrono::system_clock::time_point getWatermark();
    bool updateWatermark(std::chrono::system_clock::time_point timestamp);

    // Методы записи агрегатов
    bool writePageViews(const std::vector<AggregatedPageViews>& data);
    bool writeClicks(const std::vector<AggregatedClicks>& data);
    bool writePerformance(const std::vector<AggregatedPerformance>& data);
    bool writeErrors(const std::vector<AggregatedErrors>& data);
    bool writeCustomEvents(const std::vector<AggregatedCustomEvents>& data);

    // Записать все агрегаты
    bool writeAggregationResult(const AggregationResult& result);

    // Методы чтения агрегатов для gRPC сервера
    std::vector<AggregatedPageViews> readPageViews(
        const std::string& projectId,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to,
        const std::string& pageFilter = "",
        int limit = 1000,
        int offset = 0
    );

    std::vector<AggregatedClicks> readClicks(
        const std::string& projectId,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to,
        const std::string& pageFilter = "",
        const std::string& elementIdFilter = "",
        int limit = 1000,
        int offset = 0
    );

    std::vector<AggregatedPerformance> readPerformance(
        const std::string& projectId,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to,
        const std::string& pageFilter = "",
        int limit = 1000,
        int offset = 0
    );

    std::vector<AggregatedErrors> readErrors(
        const std::string& projectId,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to,
        const std::string& pageFilter = "",
        const std::string& errorTypeFilter = "",
        int limit = 1000,
        int offset = 0
    );

    std::vector<AggregatedCustomEvents> readCustomEvents(
        const std::string& projectId,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to,
        const std::string& eventNameFilter = "",
        const std::string& pageFilter = "",
        int limit = 1000,
        int offset = 0
    );

private:
    std::string formatTimestamp(std::chrono::system_clock::time_point tp) const;
    std::chrono::system_clock::time_point parseTimestamp(const std::string& ts) const;
    std::string escapeString(const std::string& str) const;

    PGconn* dbConnection_;
};

} // namespace aggregation

#endif // DATABASE_H

