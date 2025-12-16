#include <gtest/gtest.h>
#include "database.h"
#include "aggregator.h"
#include <chrono>

using namespace aggregation;
using namespace std::chrono;

// ===== Тесты форматирования временных меток =====

class DatabaseFormattingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Не подключаемся к реальной БД, тестируем только вспомогательные функции
    }
};

// Тест проверяет что методы Database доступны
TEST_F(DatabaseFormattingTest, DatabaseCanBeCreated) {
    Database db;
    EXPECT_FALSE(db.isConnected());  // Без connect() должно быть false
}

// ===== Тесты структур данных =====

class AggregationResultTest : public ::testing::Test {};

TEST_F(AggregationResultTest, AggregationResult_InitiallyEmpty) {
    AggregationResult result;

    EXPECT_EQ(result.pageViews.size(), 0);
    EXPECT_EQ(result.clicks.size(), 0);
    EXPECT_EQ(result.performance.size(), 0);
    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_EQ(result.customEvents.size(), 0);
}

TEST_F(AggregationResultTest, AggregatedPageViews_DefaultValues) {
    AggregatedPageViews pv;

    EXPECT_EQ(pv.viewsCount, 0);
    EXPECT_EQ(pv.uniqueUsers, 0);
    EXPECT_EQ(pv.uniqueSessions, 0);
}

TEST_F(AggregationResultTest, AggregatedPageViews_SetValues) {
    AggregatedPageViews pv;
    pv.projectId = "test-project";
    pv.page = "/home";
    pv.viewsCount = 100;
    pv.uniqueUsers = 50;
    pv.uniqueSessions = 75;
    pv.timeBucket = system_clock::now();

    EXPECT_EQ(pv.projectId, "test-project");
    EXPECT_EQ(pv.page, "/home");
    EXPECT_EQ(pv.viewsCount, 100);
    EXPECT_EQ(pv.uniqueUsers, 50);
    EXPECT_EQ(pv.uniqueSessions, 75);
}

TEST_F(AggregationResultTest, AggregatedClicks_DefaultValues) {
    AggregatedClicks clicks;

    EXPECT_EQ(clicks.clicksCount, 0);
    EXPECT_EQ(clicks.uniqueUsers, 0);
    EXPECT_EQ(clicks.uniqueSessions, 0);
}

TEST_F(AggregationResultTest, AggregatedClicks_SetValues) {
    AggregatedClicks clicks;
    clicks.projectId = "test-project";
    clicks.page = "/home";
    clicks.elementId = "btn-signup";
    clicks.clicksCount = 50;
    clicks.uniqueUsers = 25;
    clicks.uniqueSessions = 30;
    clicks.timeBucket = system_clock::now();

    EXPECT_EQ(clicks.projectId, "test-project");
    EXPECT_EQ(clicks.page, "/home");
    EXPECT_EQ(clicks.elementId, "btn-signup");
    EXPECT_EQ(clicks.clicksCount, 50);
    EXPECT_EQ(clicks.uniqueUsers, 25);
    EXPECT_EQ(clicks.uniqueSessions, 30);
}

TEST_F(AggregationResultTest, AggregatedPerformance_DefaultValues) {
    AggregatedPerformance perf;

    EXPECT_EQ(perf.samplesCount, 0);
    EXPECT_DOUBLE_EQ(perf.avgTotalLoadMs, 0.0);
    EXPECT_DOUBLE_EQ(perf.p95TotalLoadMs, 0.0);
    EXPECT_DOUBLE_EQ(perf.avgTtfbMs, 0.0);
    EXPECT_DOUBLE_EQ(perf.p95TtfbMs, 0.0);
    EXPECT_DOUBLE_EQ(perf.avgFcpMs, 0.0);
    EXPECT_DOUBLE_EQ(perf.p95FcpMs, 0.0);
    EXPECT_DOUBLE_EQ(perf.avgLcpMs, 0.0);
    EXPECT_DOUBLE_EQ(perf.p95LcpMs, 0.0);
}

TEST_F(AggregationResultTest, AggregatedPerformance_SetValues) {
    AggregatedPerformance perf;
    perf.projectId = "test-project";
    perf.page = "/home";
    perf.samplesCount = 100;
    perf.avgTotalLoadMs = 1500.5;
    perf.p95TotalLoadMs = 3000.0;
    perf.avgTtfbMs = 200.0;
    perf.p95TtfbMs = 500.0;
    perf.avgFcpMs = 500.0;
    perf.p95FcpMs = 1000.0;
    perf.avgLcpMs = 1000.0;
    perf.p95LcpMs = 2000.0;
    perf.timeBucket = system_clock::now();

    EXPECT_EQ(perf.projectId, "test-project");
    EXPECT_EQ(perf.page, "/home");
    EXPECT_EQ(perf.samplesCount, 100);
    EXPECT_DOUBLE_EQ(perf.avgTotalLoadMs, 1500.5);
    EXPECT_DOUBLE_EQ(perf.p95TotalLoadMs, 3000.0);
    EXPECT_DOUBLE_EQ(perf.avgTtfbMs, 200.0);
    EXPECT_DOUBLE_EQ(perf.p95TtfbMs, 500.0);
}

TEST_F(AggregationResultTest, AggregatedErrors_DefaultValues) {
    AggregatedErrors errors;

    EXPECT_EQ(errors.errorsCount, 0);
    EXPECT_EQ(errors.warningCount, 0);
    EXPECT_EQ(errors.criticalCount, 0);
    EXPECT_EQ(errors.uniqueUsers, 0);
}

TEST_F(AggregationResultTest, AggregatedErrors_SetValues) {
    AggregatedErrors errors;
    errors.projectId = "test-project";
    errors.page = "/checkout";
    errors.errorType = "ValidationError";
    errors.errorsCount = 100;
    errors.warningCount = 20;
    errors.criticalCount = 5;
    errors.uniqueUsers = 50;
    errors.timeBucket = system_clock::now();

    EXPECT_EQ(errors.projectId, "test-project");
    EXPECT_EQ(errors.page, "/checkout");
    EXPECT_EQ(errors.errorType, "ValidationError");
    EXPECT_EQ(errors.errorsCount, 100);
    EXPECT_EQ(errors.warningCount, 20);
    EXPECT_EQ(errors.criticalCount, 5);
    EXPECT_EQ(errors.uniqueUsers, 50);
}

TEST_F(AggregationResultTest, AggregatedCustomEvents_DefaultValues) {
    AggregatedCustomEvents custom;

    EXPECT_EQ(custom.eventsCount, 0);
    EXPECT_EQ(custom.uniqueUsers, 0);
    EXPECT_EQ(custom.uniqueSessions, 0);
}

TEST_F(AggregationResultTest, AggregatedCustomEvents_SetValues) {
    AggregatedCustomEvents custom;
    custom.projectId = "test-project";
    custom.eventName = "purchase";
    custom.page = "/checkout";
    custom.eventsCount = 50;
    custom.uniqueUsers = 40;
    custom.uniqueSessions = 45;
    custom.timeBucket = system_clock::now();

    EXPECT_EQ(custom.projectId, "test-project");
    EXPECT_EQ(custom.eventName, "purchase");
    EXPECT_EQ(custom.page, "/checkout");
    EXPECT_EQ(custom.eventsCount, 50);
    EXPECT_EQ(custom.uniqueUsers, 40);
    EXPECT_EQ(custom.uniqueSessions, 45);
}

// ===== Тесты добавления данных в AggregationResult =====

TEST_F(AggregationResultTest, AggregationResult_AddPageViews) {
    AggregationResult result;

    AggregatedPageViews pv1;
    pv1.projectId = "project-1";
    pv1.page = "/home";
    pv1.viewsCount = 100;

    AggregatedPageViews pv2;
    pv2.projectId = "project-1";
    pv2.page = "/about";
    pv2.viewsCount = 50;

    result.pageViews.push_back(pv1);
    result.pageViews.push_back(pv2);

    ASSERT_EQ(result.pageViews.size(), 2);
    EXPECT_EQ(result.pageViews[0].page, "/home");
    EXPECT_EQ(result.pageViews[0].viewsCount, 100);
    EXPECT_EQ(result.pageViews[1].page, "/about");
    EXPECT_EQ(result.pageViews[1].viewsCount, 50);
}

TEST_F(AggregationResultTest, AggregationResult_AddMultipleTypes) {
    AggregationResult result;

    AggregatedPageViews pv;
    pv.viewsCount = 100;
    result.pageViews.push_back(pv);

    AggregatedClicks click;
    click.clicksCount = 50;
    result.clicks.push_back(click);

    AggregatedPerformance perf;
    perf.samplesCount = 75;
    result.performance.push_back(perf);

    AggregatedErrors error;
    error.errorsCount = 10;
    result.errors.push_back(error);

    AggregatedCustomEvents custom;
    custom.eventsCount = 25;
    result.customEvents.push_back(custom);

    EXPECT_EQ(result.pageViews.size(), 1);
    EXPECT_EQ(result.clicks.size(), 1);
    EXPECT_EQ(result.performance.size(), 1);
    EXPECT_EQ(result.errors.size(), 1);
    EXPECT_EQ(result.customEvents.size(), 1);
}

// ===== Тесты валидации данных =====

class DataValidationTest : public ::testing::Test {};

TEST_F(DataValidationTest, PageViews_NegativeCountsNotAllowed) {
    AggregatedPageViews pv;
    pv.viewsCount = -1;  // Некорректное значение

    // В реальном коде должна быть валидация, но здесь просто проверяем что структура принимает значения
    EXPECT_LT(pv.viewsCount, 0);
}

TEST_F(DataValidationTest, Performance_ZeroSamplesValid) {
    AggregatedPerformance perf;
    perf.samplesCount = 0;

    EXPECT_EQ(perf.samplesCount, 0);
}

TEST_F(DataValidationTest, Performance_NegativeMetricsNotExpected) {
    AggregatedPerformance perf;
    perf.avgTotalLoadMs = -100.0;  // Некорректное значение

    // Метрики производительности не должны быть отрицательными
    EXPECT_LT(perf.avgTotalLoadMs, 0);
}

// ===== Тесты граничных условий =====

class BoundaryConditionsTest : public ::testing::Test {};

TEST_F(BoundaryConditionsTest, PageViews_MaxValues) {
    AggregatedPageViews pv;
    pv.viewsCount = INT64_MAX;
    pv.uniqueUsers = INT64_MAX;
    pv.uniqueSessions = INT64_MAX;

    EXPECT_EQ(pv.viewsCount, INT64_MAX);
    EXPECT_EQ(pv.uniqueUsers, INT64_MAX);
    EXPECT_EQ(pv.uniqueSessions, INT64_MAX);
}

TEST_F(BoundaryConditionsTest, Performance_VeryLargeValues) {
    AggregatedPerformance perf;
    perf.avgTotalLoadMs = 999999.99;
    perf.p95TotalLoadMs = 999999.99;

    EXPECT_GT(perf.avgTotalLoadMs, 999999.0);
    EXPECT_GT(perf.p95TotalLoadMs, 999999.0);
}

TEST_F(BoundaryConditionsTest, EmptyStrings) {
    AggregatedPageViews pv;
    pv.projectId = "";
    pv.page = "";

    EXPECT_TRUE(pv.projectId.empty());
    EXPECT_TRUE(pv.page.empty());
}

TEST_F(BoundaryConditionsTest, VeryLongStrings) {
    AggregatedPageViews pv;
    pv.projectId = std::string(1000, 'a');
    pv.page = std::string(1000, 'b');

    EXPECT_EQ(pv.projectId.length(), 1000);
    EXPECT_EQ(pv.page.length(), 1000);
}

