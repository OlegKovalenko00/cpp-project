#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <vector>
#include <string>
#include <chrono>
#include <cstdint>

namespace aggregation {

class Database;

struct RawEvent {
    std::string projectId;
    std::string page;
    std::string eventType;
    double performanceMs;
    bool isError;
    std::string userId;
    std::string sessionId;
    std::chrono::system_clock::time_point timestamp;
};

struct AggregatedEvent {
    std::string projectId;
    std::string page;
    std::string eventType;
    int64_t eventsCount;
    int64_t uniqueUsers;
    int64_t uniqueSessions;
    int64_t errorsCount;
    double avgPerformanceMs;
    double p95PerformanceMs;
    std::chrono::system_clock::time_point timeBucket;
};

class Aggregator {
public:
    explicit Aggregator(Database& db);
    ~Aggregator();

    void run();

    static double calculateAverage(const std::vector<double>& values);
    static double calculateMin(const std::vector<double>& values);
    static double calculateMax(const std::vector<double>& values);
private:
    Database& database_;
};

} // namespace aggregation

#endif // AGGREGATOR_H

