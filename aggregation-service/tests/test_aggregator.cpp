#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include "aggregator.h"
#include "database.h"
#include "metrics_client.h"

using namespace aggregation;

// Вспомогательная функция для сравнения double
bool approxEqual(double a, double b, double epsilon = 0.001) {
    return std::fabs(a - b) < epsilon;
}

// Глобальные объекты для тестов (не требуют реального подключения)
Database testDb;
MetricsClient testMetricsClient("localhost", "50051");  // Не будет реально подключаться в тестах

// ============== Тесты статистических функций ==============

void testCalculateAverage() {
    std::cout << "testCalculateAverage... ";

    // Обычный случай
    std::vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0};
    assert(approxEqual(Aggregator::calculateAverage(values), 3.0));

    // Пустой вектор
    std::vector<double> empty;
    assert(Aggregator::calculateAverage(empty) == 0.0);

    // Один элемент
    std::vector<double> single = {42.0};
    assert(approxEqual(Aggregator::calculateAverage(single), 42.0));

    // Дробные числа
    std::vector<double> decimals = {1.5, 2.5, 3.5};
    assert(approxEqual(Aggregator::calculateAverage(decimals), 2.5));

    std::cout << "PASSED" << std::endl;
}

void testCalculateMin() {
    std::cout << "testCalculateMin... ";

    std::vector<double> values = {5.0, 2.0, 8.0, 1.0, 9.0};
    assert(Aggregator::calculateMin(values) == 1.0);

    // Пустой вектор
    std::vector<double> empty;
    assert(Aggregator::calculateMin(empty) == 0.0);

    // Отрицательные числа
    std::vector<double> negative = {-5.0, -2.0, -8.0};
    assert(Aggregator::calculateMin(negative) == -8.0);

    std::cout << "PASSED" << std::endl;
}

void testCalculateMax() {
    std::cout << "testCalculateMax... ";

    std::vector<double> values = {5.0, 2.0, 8.0, 1.0, 9.0};
    assert(Aggregator::calculateMax(values) == 9.0);

    // Пустой вектор
    std::vector<double> empty;
    assert(Aggregator::calculateMax(empty) == 0.0);

    // Отрицательные числа
    std::vector<double> negative = {-5.0, -2.0, -8.0};
    assert(Aggregator::calculateMax(negative) == -2.0);

    std::cout << "PASSED" << std::endl;
}

void testCalculateP95() {
    std::cout << "testCalculateP95... ";

    // 20 элементов: p95 должен быть 19-й элемент (индекс 18)
    std::vector<double> values;
    for (int i = 1; i <= 20; ++i) {
        values.push_back(static_cast<double>(i));
    }
    double p95 = Aggregator::calculateP95(values);
    assert(p95 >= 18.0 && p95 <= 20.0); // Приблизительно 19

    // Пустой вектор
    std::vector<double> empty;
    assert(Aggregator::calculateP95(empty) == 0.0);

    // Один элемент
    std::vector<double> single = {100.0};
    assert(Aggregator::calculateP95(single) == 100.0);

    std::cout << "PASSED" << std::endl;
}

// ============== Тесты агрегации событий ==============

void testAggregatePageViews() {
    std::cout << "testAggregatePageViews... ";

    Database db;
    MetricsClient metricsClient("localhost", "50051");
    Aggregator agg(db, metricsClient);

    auto now = std::chrono::system_clock::now();
    std::vector<RawEvent> events;

    // 5 page_view событий на одной странице
    for (int i = 0; i < 5; ++i) {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/home";
        e.eventType = "page_view";
        e.userId = "user-" + std::to_string(i % 3); // 3 уникальных пользователя
        e.sessionId = "session-" + std::to_string(i); // 5 уникальных сессий
        e.timestamp = now;
        events.push_back(e);
    }

    auto result = agg.aggregateEvents(events, std::chrono::minutes(5));

    assert(result.pageViews.size() == 1);
    assert(result.pageViews[0].viewsCount == 5);
    assert(result.pageViews[0].uniqueUsers == 3);
    assert(result.pageViews[0].uniqueSessions == 5);
    assert(result.pageViews[0].page == "/home");

    std::cout << "PASSED" << std::endl;
}

void testAggregateClicks() {
    std::cout << "testAggregateClicks... ";

    Database db;
    MetricsClient metricsClient("localhost", "50051");
    Aggregator agg(db, metricsClient);

    auto now = std::chrono::system_clock::now();
    std::vector<RawEvent> events;

    // 3 клика на кнопку "buy"
    for (int i = 0; i < 3; ++i) {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/product";
        e.eventType = "click";
        e.elementId = "buy-button";
        e.userId = "user-" + std::to_string(i);
        e.sessionId = "session-" + std::to_string(i);
        e.timestamp = now;
        events.push_back(e);
    }

    // 2 клика на другую кнопку
    for (int i = 0; i < 2; ++i) {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/product";
        e.eventType = "click";
        e.elementId = "add-to-cart";
        e.userId = "user-" + std::to_string(i);
        e.sessionId = "session-" + std::to_string(i);
        e.timestamp = now;
        events.push_back(e);
    }

    auto result = agg.aggregateEvents(events, std::chrono::minutes(5));

    assert(result.clicks.size() == 2); // Две группы по element_id

    // Проверяем что есть обе группы
    int buyClicks = 0, cartClicks = 0;
    for (const auto& c : result.clicks) {
        if (c.elementId == "buy-button") buyClicks = c.clicksCount;
        if (c.elementId == "add-to-cart") cartClicks = c.clicksCount;
    }
    assert(buyClicks == 3);
    assert(cartClicks == 2);

    std::cout << "PASSED" << std::endl;
}

void testAggregatePerformance() {
    std::cout << "testAggregatePerformance... ";

    Database db;
    MetricsClient metricsClient("localhost", "50051");
    Aggregator agg(db, metricsClient);

    auto now = std::chrono::system_clock::now();
    std::vector<RawEvent> events;

    // 3 performance события
    double loads[] = {100.0, 150.0, 200.0};
    double ttfbs[] = {20.0, 25.0, 30.0};

    for (int i = 0; i < 3; ++i) {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/home";
        e.eventType = "performance";
        e.totalPageLoadMs = loads[i];
        e.ttfbMs = ttfbs[i];
        e.fcpMs = 50.0 + i * 10;
        e.lcpMs = 80.0 + i * 10;
        e.userId = "user-" + std::to_string(i);
        e.timestamp = now;
        events.push_back(e);
    }

    auto result = agg.aggregateEvents(events, std::chrono::minutes(5));

    assert(result.performance.size() == 1);
    assert(result.performance[0].samplesCount == 3);
    assert(approxEqual(result.performance[0].avgTotalLoadMs, 150.0)); // (100+150+200)/3
    assert(approxEqual(result.performance[0].avgTtfbMs, 25.0));       // (20+25+30)/3

    std::cout << "PASSED" << std::endl;
}

void testAggregateErrors() {
    std::cout << "testAggregateErrors... ";

    Database db;
    MetricsClient metricsClient("localhost", "50051");
    Aggregator agg(db, metricsClient);

    auto now = std::chrono::system_clock::now();
    std::vector<RawEvent> events;

    // 2 WARNING ошибки
    for (int i = 0; i < 2; ++i) {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/checkout";
        e.eventType = "error";
        e.isError = true;
        e.errorType = "ValidationError";
        e.severity = 1; // WARNING
        e.userId = "user-" + std::to_string(i);
        e.timestamp = now;
        events.push_back(e);
    }

    // 1 CRITICAL ошибка
    {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/checkout";
        e.eventType = "error";
        e.isError = true;
        e.errorType = "ValidationError";
        e.severity = 3; // CRITICAL
        e.userId = "user-critical";
        e.timestamp = now;
        events.push_back(e);
    }

    auto result = agg.aggregateEvents(events, std::chrono::minutes(5));

    assert(result.errors.size() == 1);
    assert(result.errors[0].errorsCount == 3);
    assert(result.errors[0].warningCount == 2);
    assert(result.errors[0].criticalCount == 1);
    assert(result.errors[0].uniqueUsers == 3);

    std::cout << "PASSED" << std::endl;
}

void testAggregateCustomEvents() {
    std::cout << "testAggregateCustomEvents... ";

    Database db;
    MetricsClient metricsClient("localhost", "50051");
    Aggregator agg(db, metricsClient);

    auto now = std::chrono::system_clock::now();
    std::vector<RawEvent> events;

    // 4 custom события "purchase"
    for (int i = 0; i < 4; ++i) {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/checkout";
        e.eventType = "custom";
        e.customEventName = "purchase";
        e.userId = "user-" + std::to_string(i % 2); // 2 уникальных
        e.sessionId = "session-" + std::to_string(i);
        e.timestamp = now;
        events.push_back(e);
    }

    auto result = agg.aggregateEvents(events, std::chrono::minutes(5));

    assert(result.customEvents.size() == 1);
    assert(result.customEvents[0].eventsCount == 4);
    assert(result.customEvents[0].uniqueUsers == 2);
    assert(result.customEvents[0].uniqueSessions == 4);
    assert(result.customEvents[0].eventName == "purchase");

    std::cout << "PASSED" << std::endl;
}

void testTimeBucketGrouping() {
    std::cout << "testTimeBucketGrouping... ";

    Database db;
    MetricsClient metricsClient("localhost", "50051");
    Aggregator agg(db, metricsClient);

    auto now = std::chrono::system_clock::now();
    std::vector<RawEvent> events;

    // События в разных 5-минутных бакетах
    for (int i = 0; i < 3; ++i) {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/home";
        e.eventType = "page_view";
        e.userId = "user-1";
        e.timestamp = now; // Один бакет
        events.push_back(e);
    }

    for (int i = 0; i < 2; ++i) {
        RawEvent e;
        e.projectId = "project-1";
        e.page = "/home";
        e.eventType = "page_view";
        e.userId = "user-2";
        e.timestamp = now - std::chrono::minutes(10); // Другой бакет
        events.push_back(e);
    }

    auto result = agg.aggregateEvents(events, std::chrono::minutes(5));

    // Должно быть 2 группы (разные time buckets)
    assert(result.pageViews.size() == 2);

    std::cout << "PASSED" << std::endl;
}

void testMixedEventTypes() {
    std::cout << "testMixedEventTypes... ";

    Database db;
    MetricsClient metricsClient("localhost", "50051");
    Aggregator agg(db, metricsClient);

    auto now = std::chrono::system_clock::now();
    std::vector<RawEvent> events;

    // Разные типы событий
    RawEvent pv;
    pv.projectId = "project-1";
    pv.page = "/home";
    pv.eventType = "page_view";
    pv.userId = "user-1";
    pv.timestamp = now;
    events.push_back(pv);

    RawEvent click;
    click.projectId = "project-1";
    click.page = "/home";
    click.eventType = "click";
    click.elementId = "btn";
    click.userId = "user-1";
    click.timestamp = now;
    events.push_back(click);

    RawEvent perf;
    perf.projectId = "project-1";
    perf.page = "/home";
    perf.eventType = "performance";
    perf.totalPageLoadMs = 100.0;
    perf.userId = "user-1";
    perf.timestamp = now;
    events.push_back(perf);

    RawEvent err;
    err.projectId = "project-1";
    err.page = "/home";
    err.eventType = "error";
    err.errorType = "TypeError";
    err.severity = 2;
    err.userId = "user-1";
    err.timestamp = now;
    events.push_back(err);

    RawEvent custom;
    custom.projectId = "project-1";
    custom.page = "/home";
    custom.eventType = "custom";
    custom.customEventName = "signup";
    custom.userId = "user-1";
    custom.timestamp = now;
    events.push_back(custom);

    auto result = agg.aggregateEvents(events, std::chrono::minutes(5));

    assert(result.pageViews.size() == 1);
    assert(result.clicks.size() == 1);
    assert(result.performance.size() == 1);
    assert(result.errors.size() == 1);
    assert(result.customEvents.size() == 1);

    std::cout << "PASSED" << std::endl;
}

void testEmptyEvents() {
    std::cout << "testEmptyEvents... ";

    Database db;
    MetricsClient metricsClient("localhost", "50051");
    Aggregator agg(db, metricsClient);

    std::vector<RawEvent> empty;
    auto result = agg.aggregateEvents(empty, std::chrono::minutes(5));

    assert(result.pageViews.empty());
    assert(result.clicks.empty());
    assert(result.performance.empty());
    assert(result.errors.empty());
    assert(result.customEvents.empty());

    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "\n=== Aggregator Tests ===" << std::endl;

    // Статистические функции
    testCalculateAverage();
    testCalculateMin();
    testCalculateMax();
    testCalculateP95();

    // Агрегация событий
    testAggregatePageViews();
    testAggregateClicks();
    testAggregatePerformance();
    testAggregateErrors();
    testAggregateCustomEvents();

    // Группировка и смешанные события
    testTimeBucketGrouping();
    testMixedEventTypes();
    testEmptyEvents();

    std::cout << "\n✅ All aggregator tests passed!" << std::endl;
    return 0;
}
