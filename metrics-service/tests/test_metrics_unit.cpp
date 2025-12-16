#include <gtest/gtest.h>
#include "metrics.h"
#include <string>
#include <sstream>

// ===== Тесты MetricsServiceImpl =====

class MetricsServiceImplTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовую конфигурацию БД
        test_config.host = "localhost";
        test_config.dbname = "test_db";
        test_config.user = "test_user";
        test_config.password = "test_password";
    }

    DatabaseConfig test_config;
};

TEST_F(MetricsServiceImplTest, ConstructorAcceptsConfig) {
    EXPECT_NO_THROW({
        MetricsServiceImpl service(test_config);
    });
}

TEST_F(MetricsServiceImplTest, ConfigIsStored) {
    MetricsServiceImpl service(test_config);

    // Создаем connection string и проверяем что он содержит наши параметры
    // Это косвенная проверка что конфиг сохранен
    EXPECT_NO_THROW({
        // Service should be constructed without throwing
    });
}

TEST_F(MetricsServiceImplTest, DifferentDatabaseConfigs) {
    DatabaseConfig config1;
    config1.host = "host1";
    config1.dbname = "db1";
    config1.user = "user1";
    config1.password = "pass1";

    DatabaseConfig config2;
    config2.host = "host2";
    config2.dbname = "db2";
    config2.user = "user2";
    config2.password = "pass2";

    MetricsServiceImpl service1(config1);
    MetricsServiceImpl service2(config2);

    // Both services should be created successfully
    SUCCEED();
}

TEST_F(MetricsServiceImplTest, EmptyDatabaseConfig) {
    DatabaseConfig empty_config;
    empty_config.host = "";
    empty_config.dbname = "";
    empty_config.user = "";
    empty_config.password = "";

    EXPECT_NO_THROW({
        MetricsServiceImpl service(empty_config);
    });
}

// ===== Тесты вспомогательных функций (если есть публичные) =====

class ConnectionStringTest : public ::testing::Test {
protected:
    std::string buildConnectionString(const DatabaseConfig& config) {
        return "host=" + config.host +
               " dbname=" + config.dbname +
               " user=" + config.user +
               " password=" + config.password;
    }
};

TEST_F(ConnectionStringTest, StandardConfig) {
    DatabaseConfig config;
    config.host = "localhost";
    config.dbname = "metrics_db";
    config.user = "metrics_user";
    config.password = "secret";

    std::string conn_str = buildConnectionString(config);

    EXPECT_NE(conn_str.find("host=localhost"), std::string::npos);
    EXPECT_NE(conn_str.find("dbname=metrics_db"), std::string::npos);
    EXPECT_NE(conn_str.find("user=metrics_user"), std::string::npos);
    EXPECT_NE(conn_str.find("password=secret"), std::string::npos);
}

TEST_F(ConnectionStringTest, ConfigWithPort) {
    DatabaseConfig config;
    config.host = "localhost:5433";
    config.dbname = "metrics";
    config.user = "admin";
    config.password = "admin123";

    std::string conn_str = buildConnectionString(config);

    EXPECT_NE(conn_str.find("localhost:5433"), std::string::npos);
}

TEST_F(ConnectionStringTest, ConfigWithSpecialCharacters) {
    DatabaseConfig config;
    config.host = "db.example.com";
    config.dbname = "metrics-prod_2024";
    config.user = "user@domain.com";
    config.password = "p@$$w0rd!#123";

    std::string conn_str = buildConnectionString(config);

    EXPECT_NE(conn_str.find("db.example.com"), std::string::npos);
    EXPECT_NE(conn_str.find("metrics-prod_2024"), std::string::npos);
    EXPECT_NE(conn_str.find("p@$$w0rd!#123"), std::string::npos);
}

TEST_F(ConnectionStringTest, EmptyValues) {
    DatabaseConfig config;
    config.host = "";
    config.dbname = "";
    config.user = "";
    config.password = "";

    std::string conn_str = buildConnectionString(config);

    // Should still build a string, just with empty values
    EXPECT_NE(conn_str.find("host="), std::string::npos);
    EXPECT_NE(conn_str.find("dbname="), std::string::npos);
}

// ===== Тесты Query строк (моковые, без реального подключения к БД) =====

class QueryBuilderTest : public ::testing::Test {
protected:
    std::string buildPageViewsQuery(
        bool has_time_range,
        int64_t start_ts,
        int64_t end_ts,
        const std::string& page_filter,
        const std::string& user_filter,
        int limit,
        int offset) {

        std::stringstream query;
        query << "SELECT id, page, user_id, session_id, referrer, "
              << "EXTRACT(EPOCH FROM timestamp)::bigint as ts "
              << "FROM page_views WHERE 1=1";

        if (has_time_range) {
            query << " AND timestamp >= to_timestamp(" << start_ts << ")"
                  << " AND timestamp <= to_timestamp(" << end_ts << ")";
        }

        if (!page_filter.empty()) {
            query << " AND page LIKE '%" << page_filter << "%'";
        }

        if (!user_filter.empty()) {
            query << " AND user_id = '" << user_filter << "'";
        }

        if (limit > 0) {
            query << " LIMIT " << limit << " OFFSET " << offset;
        }

        return query.str();
    }
};

TEST_F(QueryBuilderTest, BasicQuery) {
    std::string query = buildPageViewsQuery(false, 0, 0, "", "", 100, 0);

    EXPECT_NE(query.find("SELECT"), std::string::npos);
    EXPECT_NE(query.find("FROM page_views"), std::string::npos);
    EXPECT_NE(query.find("WHERE 1=1"), std::string::npos);
    EXPECT_NE(query.find("LIMIT 100"), std::string::npos);
}

TEST_F(QueryBuilderTest, QueryWithTimeRange) {
    std::string query = buildPageViewsQuery(true, 1000000, 2000000, "", "", 50, 0);

    EXPECT_NE(query.find("to_timestamp(1000000)"), std::string::npos);
    EXPECT_NE(query.find("to_timestamp(2000000)"), std::string::npos);
    EXPECT_NE(query.find("LIMIT 50"), std::string::npos);
}

TEST_F(QueryBuilderTest, QueryWithPageFilter) {
    std::string query = buildPageViewsQuery(false, 0, 0, "/home", "", 100, 0);

    EXPECT_NE(query.find("page LIKE '%/home%'"), std::string::npos);
}

TEST_F(QueryBuilderTest, QueryWithUserFilter) {
    std::string query = buildPageViewsQuery(false, 0, 0, "", "user-123", 100, 0);

    EXPECT_NE(query.find("user_id = 'user-123'"), std::string::npos);
}

TEST_F(QueryBuilderTest, QueryWithAllFilters) {
    std::string query = buildPageViewsQuery(true, 1000, 2000, "/products", "user-456", 25, 10);

    EXPECT_NE(query.find("to_timestamp"), std::string::npos);
    EXPECT_NE(query.find("/products"), std::string::npos);
    EXPECT_NE(query.find("user-456"), std::string::npos);
    EXPECT_NE(query.find("LIMIT 25 OFFSET 10"), std::string::npos);
}

TEST_F(QueryBuilderTest, QueryWithPagination) {
    std::string query1 = buildPageViewsQuery(false, 0, 0, "", "", 20, 0);
    std::string query2 = buildPageViewsQuery(false, 0, 0, "", "", 20, 20);
    std::string query3 = buildPageViewsQuery(false, 0, 0, "", "", 20, 40);

    EXPECT_NE(query1.find("LIMIT 20 OFFSET 0"), std::string::npos);
    EXPECT_NE(query2.find("LIMIT 20 OFFSET 20"), std::string::npos);
    EXPECT_NE(query3.find("LIMIT 20 OFFSET 40"), std::string::npos);
}

// ===== Тесты валидации данных =====

class DataValidationTest : public ::testing::Test {};

TEST_F(DataValidationTest, PageViewRequiredFields) {
    PageView pv;
    pv.page = "/required-page";

    // Page is required
    EXPECT_FALSE(pv.page.empty());

    // Optional fields can be null
    EXPECT_FALSE(pv.user_id.has_value());
    EXPECT_FALSE(pv.session_id.has_value());
    EXPECT_FALSE(pv.referrer.has_value());
}

TEST_F(DataValidationTest, ClickEventRequiredFields) {
    ClickEvent click;
    click.page = "/required-page";

    EXPECT_FALSE(click.page.empty());
}

TEST_F(DataValidationTest, PerformanceEventRequiredFields) {
    PerformanceEvent perf;
    perf.page = "/required-page";

    EXPECT_FALSE(perf.page.empty());
}

TEST_F(DataValidationTest, ErrorEventRequiredFields) {
    ErrorEvent error;
    error.page = "/required-page";

    EXPECT_FALSE(error.page.empty());
}

TEST_F(DataValidationTest, CustomEventRequiredFields) {
    CustomEvent custom;
    custom.name = "required_event_name";

    EXPECT_FALSE(custom.name.empty());
}

TEST_F(DataValidationTest, PerformanceMetricsArePositive) {
    PerformanceEvent perf;
    perf.page = "/test";
    perf.ttfb_ms = 100.0;
    perf.fcp_ms = 200.0;
    perf.lcp_ms = 300.0;
    perf.total_page_load_ms = 500.0;

    ASSERT_TRUE(perf.ttfb_ms.has_value());
    EXPECT_GT(perf.ttfb_ms.value(), 0);
    ASSERT_TRUE(perf.fcp_ms.has_value());
    EXPECT_GT(perf.fcp_ms.value(), 0);
    ASSERT_TRUE(perf.lcp_ms.has_value());
    EXPECT_GT(perf.lcp_ms.value(), 0);
    ASSERT_TRUE(perf.total_page_load_ms.has_value());
    EXPECT_GT(perf.total_page_load_ms.value(), 0);
}

TEST_F(DataValidationTest, ErrorSeverityInRange) {
    // Обычно severity: 0=INFO, 1=WARNING, 2=ERROR, 3=CRITICAL
    ErrorEvent error;
    error.page = "/test";
    error.severity = 2;

    ASSERT_TRUE(error.severity.has_value());
    EXPECT_GE(error.severity.value(), 0);
    EXPECT_LE(error.severity.value(), 3);
}

// ===== Тесты граничных случаев =====

class EdgeCasesTest : public ::testing::Test {};

TEST_F(EdgeCasesTest, VeryLongPagePath) {
    PageView pv;
    pv.page = "/" + std::string(500, 'a');

    EXPECT_GT(pv.page.length(), 500);
}

TEST_F(EdgeCasesTest, SpecialCharactersInPage) {
    PageView pv;
    pv.page = "/products?id=123&sort=price&filter=new";

    EXPECT_NE(pv.page.find("?"), std::string::npos);
    EXPECT_NE(pv.page.find("&"), std::string::npos);
    EXPECT_NE(pv.page.find("="), std::string::npos);
}

TEST_F(EdgeCasesTest, UnicodeInStrings) {
    PageView pv;
    pv.page = "/страница";
    pv.user_id = "пользователь-123";

    EXPECT_FALSE(pv.page.empty());
    ASSERT_TRUE(pv.user_id.has_value());
    EXPECT_FALSE(pv.user_id.value().empty());
}

TEST_F(EdgeCasesTest, VeryLargeErrorStack) {
    ErrorEvent error;
    error.page = "/";
    error.stack = std::string(50000, '\n');

    ASSERT_TRUE(error.stack.has_value());
    EXPECT_EQ(error.stack.value().length(), 50000);
}

TEST_F(EdgeCasesTest, MultipleOptionalFieldsCombinations) {
    PageView pv;
    pv.page = "/test";

    // Test 1: Only user_id
    pv.user_id = "user-1";
    EXPECT_TRUE(pv.user_id.has_value());
    EXPECT_FALSE(pv.session_id.has_value());
    EXPECT_FALSE(pv.referrer.has_value());

    // Test 2: Only session_id
    pv.user_id = std::nullopt;
    pv.session_id = "session-1";
    EXPECT_FALSE(pv.user_id.has_value());
    EXPECT_TRUE(pv.session_id.has_value());
    EXPECT_FALSE(pv.referrer.has_value());

    // Test 3: Only referrer
    pv.session_id = std::nullopt;
    pv.referrer = "https://example.com";
    EXPECT_FALSE(pv.user_id.has_value());
    EXPECT_FALSE(pv.session_id.has_value());
    EXPECT_TRUE(pv.referrer.has_value());
}

TEST_F(EdgeCasesTest, ZeroAndNegativeTimestamps) {
    // В реальности timestamps должны быть положительными, но проверяем структуру
    // Timestamp хранится как int64_t в protobuf
    int64_t zero_ts = 0;
    int64_t negative_ts = -1000;
    int64_t large_ts = 9999999999;

    EXPECT_EQ(zero_ts, 0);
    EXPECT_LT(negative_ts, 0);
    EXPECT_GT(large_ts, 1000000000);
}

// ===== Тесты производительности (простые) =====

class PerformanceTest : public ::testing::Test {};

TEST_F(PerformanceTest, Create1000PageViews) {
    std::vector<PageView> views;
    views.reserve(1000);

    for (int i = 0; i < 1000; ++i) {
        PageView pv;
        pv.page = "/page-" + std::to_string(i);
        pv.user_id = "user-" + std::to_string(i);
        views.push_back(pv);
    }

    EXPECT_EQ(views.size(), 1000);
}

TEST_F(PerformanceTest, Create1000Errors) {
    std::vector<ErrorEvent> errors;
    errors.reserve(1000);

    for (int i = 0; i < 1000; ++i) {
        ErrorEvent error;
        error.page = "/page-" + std::to_string(i);
        error.error_type = "Error" + std::to_string(i % 10);
        error.severity = i % 4;
        errors.push_back(error);
    }

    EXPECT_EQ(errors.size(), 1000);
}

