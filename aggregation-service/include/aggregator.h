#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include <map>

namespace aggregation {

class Database;

struct RawEvent {
    std::string projectId;
    std::string page;
    std::string eventType;  // "page_view", "click", "performance", "error", "custom"

    // Performance metrics
    double totalPageLoadMs = 0.0;
    double ttfbMs = 0.0;
    double fcpMs = 0.0;
    double lcpMs = 0.0;

    bool isError = false;
    std::string userId;
    std::string sessionId;
    std::chrono::system_clock::time_point timestamp;

    // Click fields
    std::string elementId;

    // Error fields
    std::string errorType;
    std::string errorMessage;
    int severity = 0;  // 0=UNSPECIFIED, 1=WARNING, 2=ERROR, 3=CRITICAL

    // Custom event fields
    std::string customEventName;
    std::map<std::string, std::string> properties;
};

struct AggregatedPageViews {
    std::string projectId;
    std::string page;
    std::chrono::system_clock::time_point timeBucket;
    int64_t viewsCount = 0;
    int64_t uniqueUsers = 0;
    int64_t uniqueSessions = 0;
};

struct AggregatedClicks {
    std::string projectId;
    std::string page;
    std::string elementId;
    std::chrono::system_clock::time_point timeBucket;
    int64_t clicksCount = 0;
    int64_t uniqueUsers = 0;
    int64_t uniqueSessions = 0;
};

struct AggregatedPerformance {
    std::string projectId;
    std::string page;
    std::chrono::system_clock::time_point timeBucket;
    int64_t samplesCount = 0;
    double avgTotalLoadMs = 0.0;
    double p95TotalLoadMs = 0.0;
    double avgTtfbMs = 0.0;
    double p95TtfbMs = 0.0;
    double avgFcpMs = 0.0;
    double p95FcpMs = 0.0;
    double avgLcpMs = 0.0;
    double p95LcpMs = 0.0;
};

struct AggregatedErrors {
    std::string projectId;
    std::string page;
    std::string errorType;
    std::chrono::system_clock::time_point timeBucket;
    int64_t errorsCount = 0;
    int64_t warningCount = 0;
    int64_t criticalCount = 0;
    int64_t uniqueUsers = 0;
};

struct AggregatedCustomEvents {
    std::string projectId;
    std::string eventName;
    std::string page;
    std::chrono::system_clock::time_point timeBucket;
    int64_t eventsCount = 0;
    int64_t uniqueUsers = 0;
    int64_t uniqueSessions = 0;
};

// Контейнер для всех типов агрегатов
struct AggregationResult {
    std::vector<AggregatedPageViews> pageViews;
    std::vector<AggregatedClicks> clicks;
    std::vector<AggregatedPerformance> performance;
    std::vector<AggregatedErrors> errors;
    std::vector<AggregatedCustomEvents> customEvents;
};

class Aggregator {
public:
    explicit Aggregator(Database& db);
    ~Aggregator();

    void run();

    AggregationResult aggregateEvents(
        const std::vector<RawEvent>& events,
        std::chrono::minutes bucketSize
    );

    static double calculateAverage(const std::vector<double>& values);
    static double calculateMin(const std::vector<double>& values);
    static double calculateMax(const std::vector<double>& values);
    static double calculateP95(std::vector<double> values);
private:
    Database& database_;
};

} // namespace aggregation

#endif // AGGREGATOR_H
