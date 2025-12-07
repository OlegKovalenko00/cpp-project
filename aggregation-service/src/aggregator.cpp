#include "aggregator.h"
#include "database.h"

#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace aggregation {

Aggregator::Aggregator(Database& db) : database_(db) {
}

Aggregator::~Aggregator() = default;

void Aggregator::run() {
    std::cout << "Aggregator::run() started" << std::endl;

    // 1. Получаем watermark - с какого времени агрегировать
    auto watermark = database_.getWatermark();
    auto now = std::chrono::system_clock::now();

    std::cout << "Watermark: last aggregated at epoch + "
              << std::chrono::duration_cast<std::chrono::seconds>(watermark.time_since_epoch()).count()
              << " seconds" << std::endl;

    // 2. TODO: Когда metrics-service будет готов, заменить на:
    // MetricsClient client("localhost", "50051");
    // auto rawEvents = client.fetchAllEvents(watermark, now);

    // Пока создаём тестовые события для проверки
    std::vector<RawEvent> testEvents;

    // Тестовые page_view события
    for (int i = 0; i < 5; ++i) {
        RawEvent e;
        e.projectId = "test-project";
        e.page = "/home";
        e.eventType = "page_view";
        e.userId = "user-" + std::to_string(i % 3);
        e.sessionId = "session-" + std::to_string(i);
        e.timestamp = now - std::chrono::minutes(i);
        testEvents.push_back(e);
    }

    // Тестовые performance события
    for (int i = 0; i < 3; ++i) {
        RawEvent e;
        e.projectId = "test-project";
        e.page = "/home";
        e.eventType = "performance";
        e.userId = "user-" + std::to_string(i);
        e.sessionId = "session-perf-" + std::to_string(i);
        e.timestamp = now - std::chrono::minutes(i);
        e.totalPageLoadMs = 100.0 + i * 50.0;
        e.ttfbMs = 20.0 + i * 5.0;
        e.fcpMs = 50.0 + i * 10.0;
        e.lcpMs = 80.0 + i * 15.0;
        testEvents.push_back(e);
    }

    // Тестовые error события
    {
        RawEvent e;
        e.projectId = "test-project";
        e.page = "/checkout";
        e.eventType = "error";
        e.isError = true;
        e.errorType = "NetworkError";
        e.errorMessage = "Failed to fetch";
        e.severity = 2; // ERROR
        e.userId = "user-1";
        e.timestamp = now;
        testEvents.push_back(e);
    }

    std::cout << "Created " << testEvents.size() << " test events" << std::endl;

    // 3. Агрегируем события (5-минутные бакеты)
    auto result = aggregateEvents(testEvents, std::chrono::minutes(5));

    // 4. Записываем в БД
    bool success = database_.writeAggregationResult(result);

    if (success) {
        // 5. Обновляем watermark
        database_.updateWatermark(now);
        std::cout << "Aggregation completed successfully. Watermark updated." << std::endl;
    } else {
        std::cerr << "Failed to write aggregation results to database." << std::endl;
    }
}

// Вспомогательная функция для округления времени до bucket
static std::chrono::system_clock::time_point truncateToBucket(
    std::chrono::system_clock::time_point tp,
    std::chrono::minutes bucketSize
) {
    auto duration = tp.time_since_epoch();
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    auto bucketMinutes = (minutes.count() / bucketSize.count()) * bucketSize.count();
    return std::chrono::system_clock::time_point(std::chrono::minutes(bucketMinutes));
}

// Ключ для группировки событий
struct AggregationKey {
    std::string projectId;
    std::string page;
    std::chrono::system_clock::time_point timeBucket;
    std::string extra;  // element_id, error_type, event_name в зависимости от типа

    bool operator==(const AggregationKey& other) const {
        return projectId == other.projectId &&
               page == other.page &&
               timeBucket == other.timeBucket &&
               extra == other.extra;
    }
};

struct AggregationKeyHash {
    std::size_t operator()(const AggregationKey& k) const {
        auto h1 = std::hash<std::string>{}(k.projectId);
        auto h2 = std::hash<std::string>{}(k.page);
        auto h3 = std::hash<int64_t>{}(k.timeBucket.time_since_epoch().count());
        auto h4 = std::hash<std::string>{}(k.extra);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

AggregationResult Aggregator::aggregateEvents(
    const std::vector<RawEvent>& events,
    std::chrono::minutes bucketSize
) {
    AggregationResult result;

    // Группировки для каждого типа
    std::unordered_map<AggregationKey, std::vector<const RawEvent*>, AggregationKeyHash> pageViewGroups;
    std::unordered_map<AggregationKey, std::vector<const RawEvent*>, AggregationKeyHash> clickGroups;
    std::unordered_map<AggregationKey, std::vector<const RawEvent*>, AggregationKeyHash> perfGroups;
    std::unordered_map<AggregationKey, std::vector<const RawEvent*>, AggregationKeyHash> errorGroups;
    std::unordered_map<AggregationKey, std::vector<const RawEvent*>, AggregationKeyHash> customGroups;

    // Распределяем события по группам
    for (const auto& event : events) {
        auto bucket = truncateToBucket(event.timestamp, bucketSize);

        if (event.eventType == "page_view") {
            AggregationKey key{event.projectId, event.page, bucket, ""};
            pageViewGroups[key].push_back(&event);
        }
        else if (event.eventType == "click") {
            AggregationKey key{event.projectId, event.page, bucket, event.elementId};
            clickGroups[key].push_back(&event);
        }
        else if (event.eventType == "performance") {
            AggregationKey key{event.projectId, event.page, bucket, ""};
            perfGroups[key].push_back(&event);
        }
        else if (event.eventType == "error") {
            AggregationKey key{event.projectId, event.page, bucket, event.errorType};
            errorGroups[key].push_back(&event);
        }
        else if (event.eventType == "custom") {
            AggregationKey key{event.projectId, event.page, bucket, event.customEventName};
            customGroups[key].push_back(&event);
        }
    }

    // Агрегируем page_views
    for (const auto& [key, evts] : pageViewGroups) {
        std::unordered_set<std::string> users, sessions;
        for (const auto* e : evts) {
            if (!e->userId.empty()) users.insert(e->userId);
            if (!e->sessionId.empty()) sessions.insert(e->sessionId);
        }

        AggregatedPageViews agg;
        agg.projectId = key.projectId;
        agg.page = key.page;
        agg.timeBucket = key.timeBucket;
        agg.viewsCount = static_cast<int64_t>(evts.size());
        agg.uniqueUsers = static_cast<int64_t>(users.size());
        agg.uniqueSessions = static_cast<int64_t>(sessions.size());
        result.pageViews.push_back(agg);
    }

    // Агрегируем clicks
    for (const auto& [key, evts] : clickGroups) {
        std::unordered_set<std::string> users, sessions;
        for (const auto* e : evts) {
            if (!e->userId.empty()) users.insert(e->userId);
            if (!e->sessionId.empty()) sessions.insert(e->sessionId);
        }

        AggregatedClicks agg;
        agg.projectId = key.projectId;
        agg.page = key.page;
        agg.elementId = key.extra;
        agg.timeBucket = key.timeBucket;
        agg.clicksCount = static_cast<int64_t>(evts.size());
        agg.uniqueUsers = static_cast<int64_t>(users.size());
        agg.uniqueSessions = static_cast<int64_t>(sessions.size());
        result.clicks.push_back(agg);
    }

    // Агрегируем performance
    for (const auto& [key, evts] : perfGroups) {
        std::vector<double> totalLoads, ttfbs, fcps, lcps;

        for (const auto* e : evts) {
            if (e->totalPageLoadMs > 0) totalLoads.push_back(e->totalPageLoadMs);
            if (e->ttfbMs > 0) ttfbs.push_back(e->ttfbMs);
            if (e->fcpMs > 0) fcps.push_back(e->fcpMs);
            if (e->lcpMs > 0) lcps.push_back(e->lcpMs);
        }

        AggregatedPerformance agg;
        agg.projectId = key.projectId;
        agg.page = key.page;
        agg.timeBucket = key.timeBucket;
        agg.samplesCount = static_cast<int64_t>(evts.size());
        agg.avgTotalLoadMs = calculateAverage(totalLoads);
        agg.p95TotalLoadMs = calculateP95(totalLoads);
        agg.avgTtfbMs = calculateAverage(ttfbs);
        agg.p95TtfbMs = calculateP95(ttfbs);
        agg.avgFcpMs = calculateAverage(fcps);
        agg.p95FcpMs = calculateP95(fcps);
        agg.avgLcpMs = calculateAverage(lcps);
        agg.p95LcpMs = calculateP95(lcps);
        result.performance.push_back(agg);
    }

    // Агрегируем errors
    for (const auto& [key, evts] : errorGroups) {
        std::unordered_set<std::string> users;
        int64_t warningCount = 0, criticalCount = 0;

        for (const auto* e : evts) {
            if (!e->userId.empty()) users.insert(e->userId);
            if (e->severity == 1) warningCount++;        // SEVERITY_WARNING
            else if (e->severity == 3) criticalCount++;  // SEVERITY_CRITICAL
        }

        AggregatedErrors agg;
        agg.projectId = key.projectId;
        agg.page = key.page;
        agg.errorType = key.extra;
        agg.timeBucket = key.timeBucket;
        agg.errorsCount = static_cast<int64_t>(evts.size());
        agg.warningCount = warningCount;
        agg.criticalCount = criticalCount;
        agg.uniqueUsers = static_cast<int64_t>(users.size());
        result.errors.push_back(agg);
    }

    // Агрегируем custom events
    for (const auto& [key, evts] : customGroups) {
        std::unordered_set<std::string> users, sessions;
        for (const auto* e : evts) {
            if (!e->userId.empty()) users.insert(e->userId);
            if (!e->sessionId.empty()) sessions.insert(e->sessionId);
        }

        AggregatedCustomEvents agg;
        agg.projectId = key.projectId;
        agg.page = key.page;
        agg.eventName = key.extra;
        agg.timeBucket = key.timeBucket;
        agg.eventsCount = static_cast<int64_t>(evts.size());
        agg.uniqueUsers = static_cast<int64_t>(users.size());
        agg.uniqueSessions = static_cast<int64_t>(sessions.size());
        result.customEvents.push_back(agg);
    }

    std::cout << "Aggregation complete: "
              << result.pageViews.size() << " page_view groups, "
              << result.clicks.size() << " click groups, "
              << result.performance.size() << " performance groups, "
              << result.errors.size() << " error groups, "
              << result.customEvents.size() << " custom event groups" << std::endl;

    return result;
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
    double minVal = values[0];
    for (const auto& val : values) {
        if (val < minVal) minVal = val;
    }
    return minVal;
}

double Aggregator::calculateMax(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double maxVal = values[0];
    for (const auto& val : values) {
        if (val > maxVal) maxVal = val;
    }
    return maxVal;
}

double Aggregator::calculateP95(std::vector<double> values) {
    if (values.empty()) return 0.0;

    std::sort(values.begin(), values.end());

    size_t index = static_cast<size_t>(0.95 * static_cast<double>(values.size() - 1));
    return values[index];
}

} // namespace aggregation
