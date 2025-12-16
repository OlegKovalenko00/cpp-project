#include "message_processor.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "test_utils.hpp"

class MessageProcessorTest : public ::testing::Test {
protected:
    DatabaseConfig config = TestDatabaseConfig();

    void SetUp() override {
        ResetDatabase(config);
    }
};

TEST_F(MessageProcessorTest, ProcessesPageViewPayload) {
    nlohmann::json payload = {
        {"page", "/processor"},
        {"user_id", "processor-user"},
        {"session_id", "processor-session"},
        {"referrer", "https://ref.example"}
    };

    process_message("page_views", payload.dump(), config);

    auto rows = ExecuteQuery(config, "SELECT page, user_id, session_id FROM page_views");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["page"].as<std::string>(), "/processor");
}

TEST_F(MessageProcessorTest, ProcessesClickPayload) {
    nlohmann::json payload = {
        {"page", "/processor-click"},
        {"element_id", "btn"},
        {"action", "click"}
    };

    process_message("clicks", payload.dump(), config);

    auto rows = ExecuteQuery(config, "SELECT page, element_id FROM click_events");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["element_id"].as<std::string>(), "btn");
}

TEST_F(MessageProcessorTest, ProcessesPerformancePayload) {
    nlohmann::json payload = {
        {"page", "/processor-perf"},
        {"ttfb_ms", 111.0},
        {"fcp_ms", 222.0},
        {"lcp_ms", 333.0},
        {"total_page_load_ms", 444.0}
    };

    process_message("performance_events", payload.dump(), config);

    auto rows = ExecuteQuery(config, "SELECT page, ttfb_ms FROM performance_events");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_DOUBLE_EQ(rows[0]["ttfb_ms"].as<double>(), 111.0);
}

TEST_F(MessageProcessorTest, ProcessesErrorPayload) {
    nlohmann::json payload = {
        {"page", "/processor-error"},
        {"error_type", "TypeError"},
        {"message", "boom"},
        {"severity", 3}
    };

    process_message("error_events", payload.dump(), config);

    auto rows = ExecuteQuery(config, "SELECT page, error_type, severity FROM error_events");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["error_type"].as<std::string>(), "TypeError");
    EXPECT_EQ(rows[0]["severity"].as<int>(), 3);
}

TEST_F(MessageProcessorTest, ProcessesCustomPayload) {
    nlohmann::json payload = {
        {"name", "custom-processor"},
        {"page", "/processor-custom"}
    };

    process_message("custom_events", payload.dump(), config);

    auto rows = ExecuteQuery(config, "SELECT name, page FROM custom_events");
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["name"].as<std::string>(), "custom-processor");
}

TEST_F(MessageProcessorTest, InvalidJsonDoesNotThrow) {
    ASSERT_NO_THROW(process_message("page_views", "{invalid json", config));
    auto rows = ExecuteQuery(config, "SELECT * FROM page_views");
    EXPECT_EQ(rows.size(), 0);
}
