#include <gtest/gtest.h>
#include "database.h"
#include <string>
#include <optional>

// ===== Тесты DatabaseConfig =====

class DatabaseConfigTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DatabaseConfigTest, DefaultInitialization) {
    DatabaseConfig config;
    config.host = "localhost";
    config.dbname = "test_db";
    config.user = "test_user";
    config.password = "test_password";

    EXPECT_EQ(config.host, "localhost");
    EXPECT_EQ(config.dbname, "test_db");
    EXPECT_EQ(config.user, "test_user");
    EXPECT_EQ(config.password, "test_password");
}

TEST_F(DatabaseConfigTest, EmptyConfig) {
    DatabaseConfig config;
    config.host = "";
    config.dbname = "";
    config.user = "";
    config.password = "";

    EXPECT_TRUE(config.host.empty());
    EXPECT_TRUE(config.dbname.empty());
    EXPECT_TRUE(config.user.empty());
    EXPECT_TRUE(config.password.empty());
}

TEST_F(DatabaseConfigTest, ConfigWithSpecialCharacters) {
    DatabaseConfig config;
    config.host = "localhost:5432";
    config.dbname = "test-db_2024";
    config.user = "user@domain.com";
    config.password = "p@$$w0rd!";

    EXPECT_EQ(config.host, "localhost:5432");
    EXPECT_EQ(config.dbname, "test-db_2024");
    EXPECT_EQ(config.user, "user@domain.com");
    EXPECT_EQ(config.password, "p@$$w0rd!");
}

// ===== Тесты PageView =====

class PageViewTest : public ::testing::Test {};

TEST_F(PageViewTest, RequiredFieldsOnly) {
    PageView pv;
    pv.page = "/home";

    EXPECT_EQ(pv.page, "/home");
    EXPECT_FALSE(pv.user_id.has_value());
    EXPECT_FALSE(pv.session_id.has_value());
    EXPECT_FALSE(pv.referrer.has_value());
}

TEST_F(PageViewTest, AllFieldsSet) {
    PageView pv;
    pv.page = "/products";
    pv.user_id = "user-123";
    pv.session_id = "session-456";
    pv.referrer = "https://google.com";

    EXPECT_EQ(pv.page, "/products");
    ASSERT_TRUE(pv.user_id.has_value());
    EXPECT_EQ(pv.user_id.value(), "user-123");
    ASSERT_TRUE(pv.session_id.has_value());
    EXPECT_EQ(pv.session_id.value(), "session-456");
    ASSERT_TRUE(pv.referrer.has_value());
    EXPECT_EQ(pv.referrer.value(), "https://google.com");
}

TEST_F(PageViewTest, OptionalFieldsNull) {
    PageView pv;
    pv.page = "/about";
    pv.user_id = std::nullopt;
    pv.session_id = std::nullopt;
    pv.referrer = std::nullopt;

    EXPECT_EQ(pv.page, "/about");
    EXPECT_FALSE(pv.user_id.has_value());
    EXPECT_FALSE(pv.session_id.has_value());
    EXPECT_FALSE(pv.referrer.has_value());
}

TEST_F(PageViewTest, EmptyStrings) {
    PageView pv;
    pv.page = "";
    pv.user_id = "";
    pv.session_id = "";
    pv.referrer = "";

    EXPECT_TRUE(pv.page.empty());
    ASSERT_TRUE(pv.user_id.has_value());
    EXPECT_TRUE(pv.user_id.value().empty());
}

TEST_F(PageViewTest, VeryLongPage) {
    PageView pv;
    pv.page = std::string(1000, 'a');

    EXPECT_EQ(pv.page.length(), 1000);
}

// ===== Тесты ClickEvent =====

class ClickEventTest : public ::testing::Test {};

TEST_F(ClickEventTest, RequiredFieldsOnly) {
    ClickEvent click;
    click.page = "/checkout";

    EXPECT_EQ(click.page, "/checkout");
    EXPECT_FALSE(click.element_id.has_value());
    EXPECT_FALSE(click.action.has_value());
    EXPECT_FALSE(click.user_id.has_value());
    EXPECT_FALSE(click.session_id.has_value());
}

TEST_F(ClickEventTest, AllFieldsSet) {
    ClickEvent click;
    click.page = "/products";
    click.element_id = "btn-add-to-cart";
    click.action = "click";
    click.user_id = "user-789";
    click.session_id = "session-101";

    EXPECT_EQ(click.page, "/products");
    ASSERT_TRUE(click.element_id.has_value());
    EXPECT_EQ(click.element_id.value(), "btn-add-to-cart");
    ASSERT_TRUE(click.action.has_value());
    EXPECT_EQ(click.action.value(), "click");
    ASSERT_TRUE(click.user_id.has_value());
    EXPECT_EQ(click.user_id.value(), "user-789");
    ASSERT_TRUE(click.session_id.has_value());
    EXPECT_EQ(click.session_id.value(), "session-101");
}

TEST_F(ClickEventTest, DifferentActions) {
    std::vector<std::string> actions = {"click", "dblclick", "hover", "scroll", "submit"};

    for (const auto& action : actions) {
        ClickEvent click;
        click.page = "/form";
        click.action = action;

        ASSERT_TRUE(click.action.has_value());
        EXPECT_EQ(click.action.value(), action);
    }
}

// ===== Тесты PerformanceEvent =====

class PerformanceEventTest : public ::testing::Test {};

TEST_F(PerformanceEventTest, RequiredFieldsOnly) {
    PerformanceEvent perf;
    perf.page = "/dashboard";

    EXPECT_EQ(perf.page, "/dashboard");
    EXPECT_FALSE(perf.ttfb_ms.has_value());
    EXPECT_FALSE(perf.fcp_ms.has_value());
    EXPECT_FALSE(perf.lcp_ms.has_value());
    EXPECT_FALSE(perf.total_page_load_ms.has_value());
    EXPECT_FALSE(perf.user_id.has_value());
    EXPECT_FALSE(perf.session_id.has_value());
}

TEST_F(PerformanceEventTest, AllMetricsSet) {
    PerformanceEvent perf;
    perf.page = "/home";
    perf.ttfb_ms = 150.5;
    perf.fcp_ms = 500.0;
    perf.lcp_ms = 1200.7;
    perf.total_page_load_ms = 2500.3;
    perf.user_id = "user-321";
    perf.session_id = "session-654";

    EXPECT_EQ(perf.page, "/home");
    ASSERT_TRUE(perf.ttfb_ms.has_value());
    EXPECT_DOUBLE_EQ(perf.ttfb_ms.value(), 150.5);
    ASSERT_TRUE(perf.fcp_ms.has_value());
    EXPECT_DOUBLE_EQ(perf.fcp_ms.value(), 500.0);
    ASSERT_TRUE(perf.lcp_ms.has_value());
    EXPECT_DOUBLE_EQ(perf.lcp_ms.value(), 1200.7);
    ASSERT_TRUE(perf.total_page_load_ms.has_value());
    EXPECT_DOUBLE_EQ(perf.total_page_load_ms.value(), 2500.3);
}

TEST_F(PerformanceEventTest, ZeroMetrics) {
    PerformanceEvent perf;
    perf.page = "/";
    perf.ttfb_ms = 0.0;
    perf.fcp_ms = 0.0;
    perf.lcp_ms = 0.0;
    perf.total_page_load_ms = 0.0;

    ASSERT_TRUE(perf.ttfb_ms.has_value());
    EXPECT_DOUBLE_EQ(perf.ttfb_ms.value(), 0.0);
    ASSERT_TRUE(perf.fcp_ms.has_value());
    EXPECT_DOUBLE_EQ(perf.fcp_ms.value(), 0.0);
}

TEST_F(PerformanceEventTest, VeryLargeMetrics) {
    PerformanceEvent perf;
    perf.page = "/slow-page";
    perf.ttfb_ms = 10000.0;
    perf.fcp_ms = 20000.0;
    perf.lcp_ms = 30000.0;
    perf.total_page_load_ms = 60000.0;

    ASSERT_TRUE(perf.total_page_load_ms.has_value());
    EXPECT_GT(perf.total_page_load_ms.value(), 50000.0);
}

TEST_F(PerformanceEventTest, NegativeMetricsAccepted) {
    // В реальном коде должна быть валидация, но здесь проверяем что структура принимает
    PerformanceEvent perf;
    perf.page = "/test";
    perf.ttfb_ms = -100.0;  // Некорректное значение

    ASSERT_TRUE(perf.ttfb_ms.has_value());
    EXPECT_LT(perf.ttfb_ms.value(), 0);
}

// ===== Тесты ErrorEvent =====

class ErrorEventTest : public ::testing::Test {};

TEST_F(ErrorEventTest, RequiredFieldsOnly) {
    ErrorEvent error;
    error.page = "/checkout";

    EXPECT_EQ(error.page, "/checkout");
    EXPECT_FALSE(error.error_type.has_value());
    EXPECT_FALSE(error.message.has_value());
    EXPECT_FALSE(error.stack.has_value());
    EXPECT_FALSE(error.severity.has_value());
    EXPECT_FALSE(error.user_id.has_value());
    EXPECT_FALSE(error.session_id.has_value());
}

TEST_F(ErrorEventTest, AllFieldsSet) {
    ErrorEvent error;
    error.page = "/api/checkout";
    error.error_type = "TypeError";
    error.message = "Cannot read property 'amount' of undefined";
    error.stack = "at checkout.js:42:15\nat processPayment.js:10:3";
    error.severity = 2;  // ERROR
    error.user_id = "user-999";
    error.session_id = "session-888";

    EXPECT_EQ(error.page, "/api/checkout");
    ASSERT_TRUE(error.error_type.has_value());
    EXPECT_EQ(error.error_type.value(), "TypeError");
    ASSERT_TRUE(error.message.has_value());
    EXPECT_EQ(error.message.value(), "Cannot read property 'amount' of undefined");
    ASSERT_TRUE(error.stack.has_value());
    EXPECT_FALSE(error.stack.value().empty());
    ASSERT_TRUE(error.severity.has_value());
    EXPECT_EQ(error.severity.value(), 2);
}

TEST_F(ErrorEventTest, DifferentSeverities) {
    // 0 = INFO, 1 = WARNING, 2 = ERROR, 3 = CRITICAL
    for (int32_t sev = 0; sev <= 3; ++sev) {
        ErrorEvent error;
        error.page = "/test";
        error.severity = sev;

        ASSERT_TRUE(error.severity.has_value());
        EXPECT_EQ(error.severity.value(), sev);
    }
}

TEST_F(ErrorEventTest, VeryLongErrorMessage) {
    ErrorEvent error;
    error.page = "/";
    error.message = std::string(10000, 'x');

    ASSERT_TRUE(error.message.has_value());
    EXPECT_EQ(error.message.value().length(), 10000);
}

TEST_F(ErrorEventTest, EmptyStackTrace) {
    ErrorEvent error;
    error.page = "/test";
    error.error_type = "CustomError";
    error.stack = "";

    ASSERT_TRUE(error.stack.has_value());
    EXPECT_TRUE(error.stack.value().empty());
}

// ===== Тесты CustomEvent =====

class CustomEventTest : public ::testing::Test {};

TEST_F(CustomEventTest, RequiredFieldsOnly) {
    CustomEvent custom;
    custom.name = "button_clicked";

    EXPECT_EQ(custom.name, "button_clicked");
    EXPECT_FALSE(custom.page.has_value());
    EXPECT_FALSE(custom.user_id.has_value());
    EXPECT_FALSE(custom.session_id.has_value());
}

TEST_F(CustomEventTest, AllFieldsSet) {
    CustomEvent custom;
    custom.name = "purchase_completed";
    custom.page = "/checkout/success";
    custom.user_id = "user-555";
    custom.session_id = "session-777";

    EXPECT_EQ(custom.name, "purchase_completed");
    ASSERT_TRUE(custom.page.has_value());
    EXPECT_EQ(custom.page.value(), "/checkout/success");
    ASSERT_TRUE(custom.user_id.has_value());
    EXPECT_EQ(custom.user_id.value(), "user-555");
    ASSERT_TRUE(custom.session_id.has_value());
    EXPECT_EQ(custom.session_id.value(), "session-777");
}

TEST_F(CustomEventTest, DifferentEventNames) {
    std::vector<std::string> event_names = {
        "signup",
        "login",
        "logout",
        "purchase",
        "download",
        "share",
        "like",
        "comment"
    };

    for (const auto& name : event_names) {
        CustomEvent custom;
        custom.name = name;

        EXPECT_EQ(custom.name, name);
    }
}

TEST_F(CustomEventTest, EventNameWithUnderscores) {
    CustomEvent custom;
    custom.name = "product_view_from_search_results";

    EXPECT_EQ(custom.name, "product_view_from_search_results");
    EXPECT_GT(custom.name.length(), 10);
}

TEST_F(CustomEventTest, EmptyEventName) {
    CustomEvent custom;
    custom.name = "";

    EXPECT_TRUE(custom.name.empty());
}

// ===== Граничные условия =====

class BoundaryConditionsTest : public ::testing::Test {};

TEST_F(BoundaryConditionsTest, MaxInt32Severity) {
    ErrorEvent error;
    error.page = "/";
    error.severity = INT32_MAX;

    ASSERT_TRUE(error.severity.has_value());
    EXPECT_EQ(error.severity.value(), INT32_MAX);
}

TEST_F(BoundaryConditionsTest, MinInt32Severity) {
    ErrorEvent error;
    error.page = "/";
    error.severity = INT32_MIN;

    ASSERT_TRUE(error.severity.has_value());
    EXPECT_EQ(error.severity.value(), INT32_MIN);
}

TEST_F(BoundaryConditionsTest, MaxDoublePerformance) {
    PerformanceEvent perf;
    perf.page = "/";
    perf.total_page_load_ms = 999999999.999;

    ASSERT_TRUE(perf.total_page_load_ms.has_value());
    EXPECT_GT(perf.total_page_load_ms.value(), 999999999.0);
}

TEST_F(BoundaryConditionsTest, VerySmallDoublePerformance) {
    PerformanceEvent perf;
    perf.page = "/";
    perf.ttfb_ms = 0.001;

    ASSERT_TRUE(perf.ttfb_ms.has_value());
    EXPECT_LT(perf.ttfb_ms.value(), 0.01);
}

