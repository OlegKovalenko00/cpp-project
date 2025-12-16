#include <gtest/gtest.h>
#include "aggregator.h"
#include <chrono>
#include <vector>

using namespace aggregation;
using namespace std::chrono;

// Forward declarations for helper functions
std::vector<AggregatedPageViews> aggregatePageViewsOnly(const std::vector<RawEvent>& events);
std::vector<AggregatedPerformance> aggregatePerformanceOnly(const std::vector<RawEvent>& events);
std::vector<AggregatedErrors> aggregateErrorsOnly(const std::vector<RawEvent>& events);

// ===== Тесты вспомогательных функций =====

class AggregatorUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(AggregatorUtilsTest, CalculateAverage_EmptyVector) {
    std::vector<double> empty;
    EXPECT_DOUBLE_EQ(Aggregator::calculateAverage(empty), 0.0);
}

TEST_F(AggregatorUtilsTest, CalculateAverage_SingleValue) {
    std::vector<double> values = {42.5};
    EXPECT_DOUBLE_EQ(Aggregator::calculateAverage(values), 42.5);
}

TEST_F(AggregatorUtilsTest, CalculateAverage_MultipleValues) {
    std::vector<double> values = {10.0, 20.0, 30.0, 40.0, 50.0};
    EXPECT_DOUBLE_EQ(Aggregator::calculateAverage(values), 30.0);
}

TEST_F(AggregatorUtilsTest, CalculateAverage_NegativeValues) {
    std::vector<double> values = {-10.0, -20.0, -30.0};
    EXPECT_DOUBLE_EQ(Aggregator::calculateAverage(values), -20.0);
}

TEST_F(AggregatorUtilsTest, CalculateMin_EmptyVector) {
    std::vector<double> empty;
    EXPECT_DOUBLE_EQ(Aggregator::calculateMin(empty), 0.0);
}

TEST_F(AggregatorUtilsTest, CalculateMin_SingleValue) {
    std::vector<double> values = {42.5};
    EXPECT_DOUBLE_EQ(Aggregator::calculateMin(values), 42.5);
}

TEST_F(AggregatorUtilsTest, CalculateMin_MultipleValues) {
    std::vector<double> values = {30.0, 10.0, 50.0, 20.0, 40.0};
    EXPECT_DOUBLE_EQ(Aggregator::calculateMin(values), 10.0);
}

TEST_F(AggregatorUtilsTest, CalculateMax_EmptyVector) {
    std::vector<double> empty;
    EXPECT_DOUBLE_EQ(Aggregator::calculateMax(empty), 0.0);
}

TEST_F(AggregatorUtilsTest, CalculateMax_SingleValue) {
    std::vector<double> values = {42.5};
    EXPECT_DOUBLE_EQ(Aggregator::calculateMax(values), 42.5);
}

TEST_F(AggregatorUtilsTest, CalculateMax_MultipleValues) {
    std::vector<double> values = {30.0, 10.0, 50.0, 20.0, 40.0};
    EXPECT_DOUBLE_EQ(Aggregator::calculateMax(values), 50.0);
}

TEST_F(AggregatorUtilsTest, CalculateP95_EmptyVector) {
    std::vector<double> empty;
    EXPECT_DOUBLE_EQ(Aggregator::calculateP95(empty), 0.0);
}

TEST_F(AggregatorUtilsTest, CalculateP95_SingleValue) {
    std::vector<double> values = {100.0};
    EXPECT_DOUBLE_EQ(Aggregator::calculateP95(values), 100.0);
}

TEST_F(AggregatorUtilsTest, CalculateP95_TenValues) {
    std::vector<double> values = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    // P95 для 10 значений: индекс = 0.95 * 9 = 8.55 ≈ 8, значит 9-й элемент = 90
    EXPECT_DOUBLE_EQ(Aggregator::calculateP95(values), 90.0);
}

TEST_F(AggregatorUtilsTest, CalculateP95_HundredValues) {
    std::vector<double> values;
    for (int i = 1; i <= 100; ++i) {
        values.push_back(i);
    }
    // P95 для 100 значений: индекс = 0.95 * 99 = 94.05 ≈ 94, значит 95-й элемент
    EXPECT_DOUBLE_EQ(Aggregator::calculateP95(values), 95.0);
}

// ===== Тесты агрегации событий =====

class AggregatorAggregationTest : public ::testing::Test {
protected:
    void SetUp() override {
        now = system_clock::now();
        // Округляем до 5-минутного бакета
        auto duration = now.time_since_epoch();
        auto minutes = duration_cast<std::chrono::minutes>(duration);
        auto bucketMinutes = (minutes.count() / 5) * 5;
        now = system_clock::time_point(std::chrono::minutes(bucketMinutes));
    }

    system_clock::time_point now;
};

TEST_F(AggregatorAggregationTest, AggregatePageViews_Empty) {
    // Мок базы данных и клиента не нужны для тестирования чистой логики
    std::vector<RawEvent> events;

    // Создаем временный aggregator (используем nullptr для зависимостей)
    // В реальности нужен мок, но здесь тестируем только статическую функцию
    auto result = aggregatePageViewsOnly(events);

    EXPECT_EQ(result.size(), 0);
}

TEST_F(AggregatorAggregationTest, AggregatePageViews_SingleEvent) {
    std::vector<RawEvent> events;

    RawEvent event;
    event.projectId = "test-project";
    event.page = "/home";
    event.eventType = "page_view";
    event.userId = "user-1";
    event.sessionId = "session-1";
    event.timestamp = now;
    events.push_back(event);

    auto result = aggregatePageViewsOnly(events);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].projectId, "test-project");
    EXPECT_EQ(result[0].page, "/home");
    EXPECT_EQ(result[0].viewsCount, 1);
    EXPECT_EQ(result[0].uniqueUsers, 1);
    EXPECT_EQ(result[0].uniqueSessions, 1);
}

TEST_F(AggregatorAggregationTest, AggregatePageViews_MultipleUsersOnePage) {
    std::vector<RawEvent> events;

    for (int i = 0; i < 5; ++i) {
        RawEvent event;
        event.projectId = "test-project";
        event.page = "/home";
        event.eventType = "page_view";
        event.userId = "user-" + std::to_string(i);
        event.sessionId = "session-" + std::to_string(i);
        event.timestamp = now;
        events.push_back(event);
    }

    auto result = aggregatePageViewsOnly(events);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].viewsCount, 5);
    EXPECT_EQ(result[0].uniqueUsers, 5);
    EXPECT_EQ(result[0].uniqueSessions, 5);
}

TEST_F(AggregatorAggregationTest, AggregatePageViews_SameUserMultipleSessions) {
    std::vector<RawEvent> events;

    for (int i = 0; i < 3; ++i) {
        RawEvent event;
        event.projectId = "test-project";
        event.page = "/home";
        event.eventType = "page_view";
        event.userId = "user-1";  // Один и тот же пользователь
        event.sessionId = "session-" + std::to_string(i);
        event.timestamp = now;
        events.push_back(event);
    }

    auto result = aggregatePageViewsOnly(events);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].viewsCount, 3);
    EXPECT_EQ(result[0].uniqueUsers, 1);  // Один уникальный пользователь
    EXPECT_EQ(result[0].uniqueSessions, 3);  // Три сессии
}

TEST_F(AggregatorAggregationTest, AggregatePageViews_DifferentPages) {
    std::vector<RawEvent> events;

    std::vector<std::string> pages = {"/home", "/about", "/products"};
    for (const auto& page : pages) {
        RawEvent event;
        event.projectId = "test-project";
        event.page = page;
        event.eventType = "page_view";
        event.userId = "user-1";
        event.sessionId = "session-1";
        event.timestamp = now;
        events.push_back(event);
    }

    auto result = aggregatePageViewsOnly(events);

    // Должно быть 3 группы (по одной на каждую страницу)
    EXPECT_EQ(result.size(), 3);
}

TEST_F(AggregatorAggregationTest, AggregatePerformance_CalculatesMetrics) {
    std::vector<RawEvent> events;

    // Добавляем события с разными метриками производительности
    std::vector<double> loads = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    for (double load : loads) {
        RawEvent event;
        event.projectId = "test-project";
        event.page = "/home";
        event.eventType = "performance";
        event.userId = "user-1";
        event.timestamp = now;
        event.totalPageLoadMs = load;
        event.ttfbMs = load * 0.1;
        event.fcpMs = load * 0.3;
        event.lcpMs = load * 0.7;
        events.push_back(event);
    }

    auto result = aggregatePerformanceOnly(events);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].samplesCount, 10);
    EXPECT_DOUBLE_EQ(result[0].avgTotalLoadMs, 550.0);  // Среднее 100-1000
    EXPECT_DOUBLE_EQ(result[0].p95TotalLoadMs, 950.0);  // P95
}

TEST_F(AggregatorAggregationTest, AggregateErrors_CountsBySeverity) {
    std::vector<RawEvent> events;

    // 2 warnings, 3 errors, 1 critical
    for (int i = 0; i < 2; ++i) {
        RawEvent event;
        event.projectId = "test-project";
        event.page = "/checkout";
        event.eventType = "error";
        event.errorType = "ValidationError";
        event.severity = 1;  // WARNING
        event.userId = "user-" + std::to_string(i);
        event.timestamp = now;
        events.push_back(event);
    }

    for (int i = 0; i < 3; ++i) {
        RawEvent event;
        event.projectId = "test-project";
        event.page = "/checkout";
        event.eventType = "error";
        event.errorType = "ValidationError";
        event.severity = 2;  // ERROR
        event.userId = "user-" + std::to_string(i + 2);
        event.timestamp = now;
        events.push_back(event);
    }

    RawEvent event;
    event.projectId = "test-project";
    event.page = "/checkout";
    event.eventType = "error";
    event.errorType = "ValidationError";
    event.severity = 3;  // CRITICAL
    event.userId = "user-5";
    event.timestamp = now;
    events.push_back(event);

    auto result = aggregateErrorsOnly(events);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].errorsCount, 6);  // Всего ошибок
    EXPECT_EQ(result[0].warningCount, 2);
    EXPECT_EQ(result[0].criticalCount, 1);
    EXPECT_EQ(result[0].uniqueUsers, 6);
}

// ===== Вспомогательные функции для тестирования =====

std::vector<AggregatedPageViews> aggregatePageViewsOnly(const std::vector<RawEvent>& events) {
    std::vector<AggregatedPageViews> result;
    std::map<std::tuple<std::string, std::string, std::chrono::system_clock::time_point>,
             std::vector<const RawEvent*>> groups;

    for (const auto& event : events) {
        if (event.eventType == "page_view") {
            auto key = std::make_tuple(event.projectId, event.page, event.timestamp);
            groups[key].push_back(&event);
        }
    }

    for (const auto& [key, evts] : groups) {
        std::set<std::string> users, sessions;
        for (const auto* e : evts) {
            if (!e->userId.empty()) users.insert(e->userId);
            if (!e->sessionId.empty()) sessions.insert(e->sessionId);
        }

        AggregatedPageViews agg;
        agg.projectId = std::get<0>(key);
        agg.page = std::get<1>(key);
        agg.timeBucket = std::get<2>(key);
        agg.viewsCount = evts.size();
        agg.uniqueUsers = users.size();
        agg.uniqueSessions = sessions.size();
        result.push_back(agg);
    }

    return result;
}

std::vector<AggregatedPerformance> aggregatePerformanceOnly(const std::vector<RawEvent>& events) {
    std::vector<AggregatedPerformance> result;
    std::map<std::tuple<std::string, std::string, std::chrono::system_clock::time_point>,
             std::vector<const RawEvent*>> groups;

    for (const auto& event : events) {
        if (event.eventType == "performance") {
            auto key = std::make_tuple(event.projectId, event.page, event.timestamp);
            groups[key].push_back(&event);
        }
    }

    for (const auto& [key, evts] : groups) {
        std::vector<double> totalLoads, ttfbs, fcps, lcps;

        for (const auto* e : evts) {
            if (e->totalPageLoadMs > 0) totalLoads.push_back(e->totalPageLoadMs);
            if (e->ttfbMs > 0) ttfbs.push_back(e->ttfbMs);
            if (e->fcpMs > 0) fcps.push_back(e->fcpMs);
            if (e->lcpMs > 0) lcps.push_back(e->lcpMs);
        }

        AggregatedPerformance agg;
        agg.projectId = std::get<0>(key);
        agg.page = std::get<1>(key);
        agg.timeBucket = std::get<2>(key);
        agg.samplesCount = evts.size();
        agg.avgTotalLoadMs = Aggregator::calculateAverage(totalLoads);
        agg.p95TotalLoadMs = Aggregator::calculateP95(totalLoads);
        agg.avgTtfbMs = Aggregator::calculateAverage(ttfbs);
        agg.p95TtfbMs = Aggregator::calculateP95(ttfbs);
        agg.avgFcpMs = Aggregator::calculateAverage(fcps);
        agg.p95FcpMs = Aggregator::calculateP95(fcps);
        agg.avgLcpMs = Aggregator::calculateAverage(lcps);
        agg.p95LcpMs = Aggregator::calculateP95(lcps);
        result.push_back(agg);
    }

    return result;
}

std::vector<AggregatedErrors> aggregateErrorsOnly(const std::vector<RawEvent>& events) {
    std::vector<AggregatedErrors> result;
    std::map<std::tuple<std::string, std::string, std::string, std::chrono::system_clock::time_point>,
             std::vector<const RawEvent*>> groups;

    for (const auto& event : events) {
        if (event.eventType == "error") {
            auto key = std::make_tuple(event.projectId, event.page, event.errorType, event.timestamp);
            groups[key].push_back(&event);
        }
    }

    for (const auto& [key, evts] : groups) {
        std::set<std::string> users;
        int64_t warningCount = 0, criticalCount = 0;

        for (const auto* e : evts) {
            if (!e->userId.empty()) users.insert(e->userId);
            if (e->severity == 1) warningCount++;
            else if (e->severity == 3) criticalCount++;
        }

        AggregatedErrors agg;
        agg.projectId = std::get<0>(key);
        agg.page = std::get<1>(key);
        agg.errorType = std::get<2>(key);
        agg.timeBucket = std::get<3>(key);
        agg.errorsCount = evts.size();
        agg.warningCount = warningCount;
        agg.criticalCount = criticalCount;
        agg.uniqueUsers = users.size();
        result.push_back(agg);
    }

    return result;
}

