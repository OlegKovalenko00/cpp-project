#include "database.h"

#include <gtest/gtest.h>
#include <pqxx/pqxx>
#include <cstdlib>

#include "test_utils.hpp"

class DatabaseWriteTest : public ::testing::Test {
protected:
    DatabaseConfig config = TestDatabaseConfig();

    void SetUp() override {
        ResetDatabase(config);
    }
};

TEST(DatabaseConfigTest, LoadsValuesFromEnvironment) {
    setenv("POSTGRES_HOST", "env-host", 1);
    setenv("POSTGRES_DB", "env-db", 1);
    setenv("POSTGRES_USER", "env-user", 1);
    setenv("POSTGRES_PASSWORD", "env-pass", 1);

    auto config = load_database_config();
    EXPECT_EQ(config.host, "env-host");
    EXPECT_EQ(config.dbname, "env-db");
    EXPECT_EQ(config.user, "env-user");
    EXPECT_EQ(config.password, "env-pass");

    setenv("POSTGRES_HOST", "localhost", 1);
    setenv("POSTGRES_DB", "metrics_db", 1);
    setenv("POSTGRES_USER", "metrics_user", 1);
    setenv("POSTGRES_PASSWORD", "metrics_password", 1);
}

TEST_F(DatabaseWriteTest, TestDatabaseConnectionSucceeds) {
    EXPECT_TRUE(test_database_connection(config));
}

TEST_F(DatabaseWriteTest, SavePageViewPersistsRow) {
    PageView event;
    event.page = "/database-test";
    event.user_id = "db-user";
    event.session_id = "db-session";
    event.referrer = "https://example.com";

    ASSERT_TRUE(save_page_view(config, event));

    auto rows = ExecuteQuery(config, "SELECT page, user_id, session_id, referrer FROM page_views");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["page"].as<std::string>(), event.page);
    EXPECT_EQ(rows[0]["user_id"].as<std::string>(), *event.user_id);
    EXPECT_EQ(rows[0]["session_id"].as<std::string>(), *event.session_id);
    EXPECT_EQ(rows[0]["referrer"].as<std::string>(), *event.referrer);
}

TEST_F(DatabaseWriteTest, SaveClickEventPersistsRow) {
    ClickEvent event;
    event.page = "/click-test";
    event.element_id = "button";
    event.action = "click";
    event.user_id = "user-click";
    event.session_id = "session-click";

    ASSERT_TRUE(save_click_event(config, event));

    auto rows = ExecuteQuery(config, "SELECT page, element_id, action, user_id, session_id FROM click_events");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["element_id"].as<std::string>(), *event.element_id);
    EXPECT_EQ(rows[0]["action"].as<std::string>(), *event.action);
}

TEST_F(DatabaseWriteTest, SavePerformanceEventPersistsRow) {
    PerformanceEvent event;
    event.page = "/perf-test";
    event.ttfb_ms = 100.5;
    event.fcp_ms = 200.0;
    event.lcp_ms = 500.25;
    event.total_page_load_ms = 700.75;
    event.user_id = "user-perf";
    event.session_id = "session-perf";

    ASSERT_TRUE(save_performance_event(config, event));

    auto rows = ExecuteQuery(config, "SELECT page, ttfb_ms, fcp_ms, lcp_ms, total_page_load_ms FROM performance_events");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_DOUBLE_EQ(rows[0]["ttfb_ms"].as<double>(), *event.ttfb_ms);
    EXPECT_DOUBLE_EQ(rows[0]["total_page_load_ms"].as<double>(), *event.total_page_load_ms);
}

TEST_F(DatabaseWriteTest, SaveErrorEventPersistsRow) {
    ErrorEvent event;
    event.page = "/error-test";
    event.error_type = "TypeError";
    event.message = "something failed";
    event.stack = "stack trace";
    event.severity = 2;
    event.user_id = "user-error";
    event.session_id = "session-error";

    ASSERT_TRUE(save_error_event(config, event));

    auto rows = ExecuteQuery(config, "SELECT page, error_type, message, severity FROM error_events");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["error_type"].as<std::string>(), *event.error_type);
    EXPECT_EQ(rows[0]["severity"].as<int>(), *event.severity);
}

TEST_F(DatabaseWriteTest, SaveCustomEventPersistsRow) {
    CustomEvent event;
    event.name = "test-event";
    event.page = "/custom";
    event.user_id = "user-custom";
    event.session_id = "session-custom";

    ASSERT_TRUE(save_custom_event(config, event));

    auto rows = ExecuteQuery(config, "SELECT name, page, user_id, session_id FROM custom_events");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["name"].as<std::string>(), event.name);
    EXPECT_EQ(rows[0]["user_id"].as<std::string>(), *event.user_id);
}
