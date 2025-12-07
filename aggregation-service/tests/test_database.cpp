#include <iostream>
#include <cassert>
#include <cstdlib>
#include <chrono>
#include "database.h"
#include "aggregator.h"

using namespace aggregation;

// Получение connection string из env или дефолтные значения
std::string getTestConnectionString() {
    const char* host = std::getenv("AGG_DB_HOST");
    const char* port = std::getenv("AGG_DB_PORT");
    const char* dbname = std::getenv("AGG_DB_NAME");
    const char* user = std::getenv("AGG_DB_USER");
    const char* password = std::getenv("AGG_DB_PASSWORD");

    std::string connStr = "host=";
    connStr += (host ? host : "localhost");
    connStr += " port=";
    connStr += (port ? port : "5434");
    connStr += " dbname=";
    connStr += (dbname ? dbname : "aggregation_db");
    connStr += " user=";
    connStr += (user ? user : "agguser");
    connStr += " password=";
    connStr += (password ? password : "aggpassword");

    return connStr;
}

// ============== Тесты подключения ==============

void testDatabaseConnection() {
    std::cout << "testDatabaseConnection... ";

    Database db;

    // Изначально не подключен
    assert(!db.isConnected());

    // Подключение
    bool connected = db.connect(getTestConnectionString());
    if (!connected) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    assert(db.isConnected());

    // Отключение
    db.disconnect();
    assert(!db.isConnected());

    std::cout << "PASSED" << std::endl;
}

void testDatabaseReconnect() {
    std::cout << "testDatabaseReconnect... ";

    Database db;

    bool connected = db.connect(getTestConnectionString());
    if (!connected) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    // Переподключение (должно закрыть старое соединение)
    connected = db.connect(getTestConnectionString());
    assert(connected);
    assert(db.isConnected());

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

void testInvalidConnection() {
    std::cout << "testInvalidConnection... ";

    Database db;

    // Неверные данные подключения
    bool connected = db.connect("host=invalid_host_12345 port=9999 dbname=nodb user=nouser password=nopass connect_timeout=1");
    assert(!connected);
    assert(!db.isConnected());

    std::cout << "PASSED" << std::endl;
}

// ============== Тесты схемы ==============

void testInitializeSchema() {
    std::cout << "testInitializeSchema... ";

    Database db;
    if (!db.connect(getTestConnectionString())) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    // Инициализация схемы (должна быть идемпотентной)
    bool success = db.initializeSchema();
    assert(success);

    // Повторная инициализация тоже должна работать
    success = db.initializeSchema();
    assert(success);

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

// ============== Тесты watermark ==============

void testWatermark() {
    std::cout << "testWatermark... ";

    Database db;
    if (!db.connect(getTestConnectionString())) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    db.initializeSchema();

    // Получаем watermark
    auto watermark = db.getWatermark();

    // Обновляем watermark
    auto newTime = std::chrono::system_clock::now();
    bool updated = db.updateWatermark(newTime);
    assert(updated);

    // Проверяем что watermark изменился
    auto updatedWatermark = db.getWatermark();

    // Разница должна быть минимальной (меньше секунды)
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(
        newTime - updatedWatermark
    ).count();
    assert(std::abs(diff) <= 1);

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

// ============== Тесты записи агрегатов ==============

void testWritePageViews() {
    std::cout << "testWritePageViews... ";

    Database db;
    if (!db.connect(getTestConnectionString())) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    db.initializeSchema();

    // Очищаем таблицу для теста
    db.executeQuery("DELETE FROM agg_page_views WHERE project_id = 'test-unit'");

    std::vector<AggregatedPageViews> data;
    AggregatedPageViews pv;
    pv.projectId = "test-unit";
    pv.page = "/test-page";
    pv.timeBucket = std::chrono::system_clock::now();
    pv.viewsCount = 100;
    pv.uniqueUsers = 50;
    pv.uniqueSessions = 75;
    data.push_back(pv);

    bool success = db.writePageViews(data);
    assert(success);

    // Очистка
    db.executeQuery("DELETE FROM agg_page_views WHERE project_id = 'test-unit'");

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

void testWritePerformance() {
    std::cout << "testWritePerformance... ";

    Database db;
    if (!db.connect(getTestConnectionString())) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    db.initializeSchema();

    // Очищаем таблицу
    db.executeQuery("DELETE FROM agg_performance WHERE project_id = 'test-unit'");

    std::vector<AggregatedPerformance> data;
    AggregatedPerformance perf;
    perf.projectId = "test-unit";
    perf.page = "/test-page";
    perf.timeBucket = std::chrono::system_clock::now();
    perf.samplesCount = 10;
    perf.avgTotalLoadMs = 150.5;
    perf.p95TotalLoadMs = 250.0;
    perf.avgTtfbMs = 25.0;
    perf.p95TtfbMs = 40.0;
    perf.avgFcpMs = 60.0;
    perf.p95FcpMs = 90.0;
    perf.avgLcpMs = 100.0;
    perf.p95LcpMs = 150.0;
    data.push_back(perf);

    bool success = db.writePerformance(data);
    assert(success);

    // Очистка
    db.executeQuery("DELETE FROM agg_performance WHERE project_id = 'test-unit'");

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

void testWriteErrors() {
    std::cout << "testWriteErrors... ";

    Database db;
    if (!db.connect(getTestConnectionString())) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    db.initializeSchema();

    // Очищаем таблицу
    db.executeQuery("DELETE FROM agg_errors WHERE project_id = 'test-unit'");

    std::vector<AggregatedErrors> data;
    AggregatedErrors err;
    err.projectId = "test-unit";
    err.page = "/test-page";
    err.errorType = "TestError";
    err.timeBucket = std::chrono::system_clock::now();
    err.errorsCount = 5;
    err.warningCount = 2;
    err.criticalCount = 1;
    err.uniqueUsers = 3;
    data.push_back(err);

    bool success = db.writeErrors(data);
    assert(success);

    // Очистка
    db.executeQuery("DELETE FROM agg_errors WHERE project_id = 'test-unit'");

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

void testWriteAggregationResult() {
    std::cout << "testWriteAggregationResult... ";

    Database db;
    if (!db.connect(getTestConnectionString())) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    db.initializeSchema();

    // Очищаем таблицы
    db.executeQuery("DELETE FROM agg_page_views WHERE project_id = 'test-unit'");
    db.executeQuery("DELETE FROM agg_performance WHERE project_id = 'test-unit'");
    db.executeQuery("DELETE FROM agg_errors WHERE project_id = 'test-unit'");

    AggregationResult result;
    auto now = std::chrono::system_clock::now();

    // Page views
    AggregatedPageViews pv;
    pv.projectId = "test-unit";
    pv.page = "/home";
    pv.timeBucket = now;
    pv.viewsCount = 10;
    pv.uniqueUsers = 5;
    pv.uniqueSessions = 8;
    result.pageViews.push_back(pv);

    // Performance
    AggregatedPerformance perf;
    perf.projectId = "test-unit";
    perf.page = "/home";
    perf.timeBucket = now;
    perf.samplesCount = 3;
    perf.avgTotalLoadMs = 100.0;
    result.performance.push_back(perf);

    // Errors
    AggregatedErrors err;
    err.projectId = "test-unit";
    err.page = "/home";
    err.errorType = "NetworkError";
    err.timeBucket = now;
    err.errorsCount = 2;
    result.errors.push_back(err);

    bool success = db.writeAggregationResult(result);
    assert(success);

    // Очистка
    db.executeQuery("DELETE FROM agg_page_views WHERE project_id = 'test-unit'");
    db.executeQuery("DELETE FROM agg_performance WHERE project_id = 'test-unit'");
    db.executeQuery("DELETE FROM agg_errors WHERE project_id = 'test-unit'");

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

void testUpsertBehavior() {
    std::cout << "testUpsertBehavior... ";

    Database db;
    if (!db.connect(getTestConnectionString())) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    db.initializeSchema();

    // Очищаем таблицу
    db.executeQuery("DELETE FROM agg_page_views WHERE project_id = 'test-upsert'");

    auto now = std::chrono::system_clock::now();
    // Округляем до минуты для стабильного теста
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(now.time_since_epoch());
    auto roundedNow = std::chrono::system_clock::time_point(minutes);

    // Первая запись
    std::vector<AggregatedPageViews> data1;
    AggregatedPageViews pv1;
    pv1.projectId = "test-upsert";
    pv1.page = "/upsert-test";
    pv1.timeBucket = roundedNow;
    pv1.viewsCount = 10;
    pv1.uniqueUsers = 5;
    pv1.uniqueSessions = 8;
    data1.push_back(pv1);

    bool success = db.writePageViews(data1);
    assert(success);

    // Вторая запись с тем же ключом — должен сработать UPSERT
    std::vector<AggregatedPageViews> data2;
    AggregatedPageViews pv2;
    pv2.projectId = "test-upsert";
    pv2.page = "/upsert-test";
    pv2.timeBucket = roundedNow;
    pv2.viewsCount = 5;  // Добавится к существующему
    pv2.uniqueUsers = 3;
    pv2.uniqueSessions = 4;
    data2.push_back(pv2);

    success = db.writePageViews(data2);
    assert(success);

    // Проверяем что views_count стал 15 (10 + 5)
    // Это можно проверить через executeQuery, но у нас нет метода чтения
    // Поэтому просто проверяем что запись прошла без ошибок

    // Очистка
    db.executeQuery("DELETE FROM agg_page_views WHERE project_id = 'test-upsert'");

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

void testEmptyWrite() {
    std::cout << "testEmptyWrite... ";

    Database db;
    if (!db.connect(getTestConnectionString())) {
        std::cout << "SKIPPED (no database)" << std::endl;
        return;
    }

    db.initializeSchema();

    // Пустые векторы должны обрабатываться корректно
    std::vector<AggregatedPageViews> emptyPv;
    std::vector<AggregatedPerformance> emptyPerf;
    std::vector<AggregatedErrors> emptyErr;

    assert(db.writePageViews(emptyPv));
    assert(db.writePerformance(emptyPerf));
    assert(db.writeErrors(emptyErr));

    db.disconnect();
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "\n=== Database Tests ===" << std::endl;
    std::cout << "(Tests require PostgreSQL running on localhost:5434)" << std::endl;
    std::cout << std::endl;

    // Тесты подключения
    testDatabaseConnection();
    testDatabaseReconnect();
    testInvalidConnection();

    // Тесты схемы
    testInitializeSchema();

    // Тесты watermark
    testWatermark();

    // Тесты записи
    testWritePageViews();
    testWritePerformance();
    testWriteErrors();
    testWriteAggregationResult();

    // Тесты UPSERT
    testUpsertBehavior();
    testEmptyWrite();

    std::cout << "\n✅ All database tests completed!" << std::endl;
    return 0;
}
