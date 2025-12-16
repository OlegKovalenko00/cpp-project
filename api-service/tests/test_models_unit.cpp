#include <gtest/gtest.h>
#include "models.hpp"
#include <string>
#include <optional>

// ===== Тесты PageViewEvent =====

class PageViewEventTest : public ::testing::Test {};

TEST_F(PageViewEventTest, DefaultConstruction) {
    PageViewEvent event;

    EXPECT_TRUE(event.page.empty());
    EXPECT_FALSE(event.user_id.has_value());
    EXPECT_FALSE(event.session_id.has_value());
    EXPECT_FALSE(event.referrer.has_value());
}

TEST_F(PageViewEventTest, SetAllFields) {
    PageViewEvent event;
    event.page = "/home";
    event.user_id = "user-123";
    event.session_id = "session-456";
    event.referrer = "https://google.com";

    EXPECT_EQ(event.page, "/home");
    ASSERT_TRUE(event.user_id.has_value());
    EXPECT_EQ(event.user_id.value(), "user-123");
    ASSERT_TRUE(event.session_id.has_value());
    EXPECT_EQ(event.session_id.value(), "session-456");
    ASSERT_TRUE(event.referrer.has_value());
    EXPECT_EQ(event.referrer.value(), "https://google.com");
}

TEST_F(PageViewEventTest, ToJson) {
    PageViewEvent event;
    event.page = "/products";
    event.user_id = "user-789";
    event.timestamp = 1234567890;

    nlohmann::json json = event;

    EXPECT_EQ(json["page"], "/products");
    EXPECT_EQ(json["user_id"], "user-789");
}

TEST_F(PageViewEventTest, FromJson) {
    nlohmann::json json = {
        {"page", "/about"},
        {"user_id", "user-111"},
        {"session_id", "sess-222"},
        {"timestamp", 1234567890}
    };

    PageViewEvent event = json.get<PageViewEvent>();

    EXPECT_EQ(event.page, "/about");
    ASSERT_TRUE(event.user_id.has_value());
    EXPECT_EQ(event.user_id.value(), "user-111");
    ASSERT_TRUE(event.session_id.has_value());
    EXPECT_EQ(event.session_id.value(), "sess-222");
}

// ===== Тесты ClickEvent =====

class ClickEventTest : public ::testing::Test {};

TEST_F(ClickEventTest, DefaultConstruction) {
    ClickEvent event;

    EXPECT_TRUE(event.page.empty());
    EXPECT_TRUE(event.element_id.empty());
    EXPECT_FALSE(event.action.has_value());
}

TEST_F(ClickEventTest, SetAllFields) {
    ClickEvent event;
    event.page = "/checkout";
    event.element_id = "btn-pay";
    event.action = "click";
    event.user_id = "user-999";
    event.timestamp = 1234567890;

    EXPECT_EQ(event.page, "/checkout");
    EXPECT_EQ(event.element_id, "btn-pay");
    ASSERT_TRUE(event.action.has_value());
    EXPECT_EQ(event.action.value(), "click");
}

TEST_F(ClickEventTest, ToJson) {
    ClickEvent event;
    event.page = "/form";
    event.element_id = "submit-btn";
    event.timestamp = 1234567890;

    nlohmann::json json = event;

    EXPECT_EQ(json["page"], "/form");
    EXPECT_EQ(json["element_id"], "submit-btn");
}

TEST_F(ClickEventTest, FromJson) {
    nlohmann::json json = {
        {"page", "/contact"},
        {"element_id", "email-input"},
        {"action", "focus"},
        {"timestamp", 1234567890}
    };

    ClickEvent event = json.get<ClickEvent>();

    EXPECT_EQ(event.page, "/contact");
    EXPECT_EQ(event.element_id, "email-input");
    ASSERT_TRUE(event.action.has_value());
    EXPECT_EQ(event.action.value(), "focus");
}

// ===== Тесты PerformanceEvent =====

class PerformanceEventTest : public ::testing::Test {};

TEST_F(PerformanceEventTest, DefaultConstruction) {
    PerformanceEvent event;

    EXPECT_TRUE(event.page.empty());
    EXPECT_FALSE(event.ttfb_ms.has_value());
    EXPECT_FALSE(event.fcp_ms.has_value());
    EXPECT_FALSE(event.lcp_ms.has_value());
    EXPECT_FALSE(event.total_page_load_ms.has_value());
}

TEST_F(PerformanceEventTest, SetAllMetrics) {
    PerformanceEvent event;
    event.page = "/dashboard";
    event.ttfb_ms = 150.5;
    event.fcp_ms = 500.0;
    event.lcp_ms = 1200.7;
    event.total_page_load_ms = 2500.3;

    EXPECT_EQ(event.page, "/dashboard");
    ASSERT_TRUE(event.ttfb_ms.has_value());
    EXPECT_DOUBLE_EQ(event.ttfb_ms.value(), 150.5);
    ASSERT_TRUE(event.fcp_ms.has_value());
    EXPECT_DOUBLE_EQ(event.fcp_ms.value(), 500.0);
    ASSERT_TRUE(event.lcp_ms.has_value());
    EXPECT_DOUBLE_EQ(event.lcp_ms.value(), 1200.7);
    ASSERT_TRUE(event.total_page_load_ms.has_value());
    EXPECT_DOUBLE_EQ(event.total_page_load_ms.value(), 2500.3);
}

TEST_F(PerformanceEventTest, ToJson) {
    PerformanceEvent event;
    event.page = "/app";
    event.ttfb_ms = 100.0;
    event.total_page_load_ms = 1500.0;
    event.timestamp = 1234567890;

    nlohmann::json json = event;

    EXPECT_EQ(json["page"], "/app");
    EXPECT_DOUBLE_EQ(json["ttfb_ms"], 100.0);
    EXPECT_DOUBLE_EQ(json["total_page_load_ms"], 1500.0);
}

TEST_F(PerformanceEventTest, FromJson) {
    nlohmann::json json = {
        {"page", "/load"},
        {"ttfb_ms", 200.5},
        {"fcp_ms", 600.0},
        {"timestamp", 1234567890}
    };

    PerformanceEvent event = json.get<PerformanceEvent>();

    EXPECT_EQ(event.page, "/load");
    ASSERT_TRUE(event.ttfb_ms.has_value());
    EXPECT_DOUBLE_EQ(event.ttfb_ms.value(), 200.5);
    ASSERT_TRUE(event.fcp_ms.has_value());
    EXPECT_DOUBLE_EQ(event.fcp_ms.value(), 600.0);
}

// ===== Тесты ErrorEvent =====

class ErrorEventTest : public ::testing::Test {};

TEST_F(ErrorEventTest, DefaultConstruction) {
    ErrorEvent event;

    EXPECT_TRUE(event.page.empty());
    EXPECT_TRUE(event.error_type.empty());
    EXPECT_TRUE(event.message.empty());
    EXPECT_FALSE(event.stack.has_value());
    EXPECT_EQ(event.severity, Severity::Error); // default value
}

TEST_F(ErrorEventTest, SetAllFields) {
    ErrorEvent event;
    event.page = "/api/checkout";
    event.error_type = "TypeError";
    event.message = "Cannot read property";
    event.stack = "at line 42";
    event.severity = Severity::Critical;
    event.timestamp = 1234567890;

    EXPECT_EQ(event.page, "/api/checkout");
    EXPECT_EQ(event.error_type, "TypeError");
    EXPECT_EQ(event.message, "Cannot read property");
    EXPECT_EQ(event.severity, Severity::Critical);
}

TEST_F(ErrorEventTest, ToJson) {
    ErrorEvent event;
    event.page = "/error";
    event.error_type = "NetworkError";
    event.message = "Connection failed";
    event.severity = Severity::Critical;
    event.timestamp = 1234567890;

    nlohmann::json json = event;

    EXPECT_EQ(json["page"], "/error");
    EXPECT_EQ(json["error_type"], "NetworkError");
    EXPECT_EQ(json["severity"], "critical");
}

TEST_F(ErrorEventTest, FromJson) {
    nlohmann::json json = {
        {"page", "/fail"},
        {"error_type", "ValidationError"},
        {"message", "Invalid input"},
        {"severity", "warning"},
        {"timestamp", 1234567890}
    };

    ErrorEvent event = json.get<ErrorEvent>();

    EXPECT_EQ(event.page, "/fail");
    EXPECT_EQ(event.error_type, "ValidationError");
    EXPECT_EQ(event.severity, Severity::Warning);
}

// ===== Тесты CustomEvent =====

class CustomEventTest : public ::testing::Test {};

TEST_F(CustomEventTest, DefaultConstruction) {
    CustomEvent event;

    EXPECT_TRUE(event.name.empty());
    EXPECT_FALSE(event.page.has_value());
}

TEST_F(CustomEventTest, SetAllFields) {
    CustomEvent event;
    event.name = "purchase_completed";
    event.page = "/checkout/success";
    event.user_id = "user-555";

    EXPECT_EQ(event.name, "purchase_completed");
    ASSERT_TRUE(event.page.has_value());
    EXPECT_EQ(event.page.value(), "/checkout/success");
}

TEST_F(CustomEventTest, ToJson) {
    CustomEvent event;
    event.name = "signup";
    event.page = "/register";
    event.timestamp = 1234567890;

    nlohmann::json json = event;

    EXPECT_EQ(json["name"], "signup");
    EXPECT_EQ(json["page"], "/register");
}

TEST_F(CustomEventTest, FromJson) {
    nlohmann::json json = {
        {"name", "download"},
        {"page", "/resources"},
        {"user_id", "user-777"},
        {"timestamp", 1234567890}
    };

    CustomEvent event = json.get<CustomEvent>();

    EXPECT_EQ(event.name, "download");
    ASSERT_TRUE(event.page.has_value());
    EXPECT_EQ(event.page.value(), "/resources");
}

// ===== Тесты валидации =====

class ValidationTest : public ::testing::Test {};

TEST_F(ValidationTest, PageViewRequiredFields) {
    PageViewEvent event;
    event.page = "/required";

    EXPECT_FALSE(event.page.empty());
    // Optional fields can be empty
    EXPECT_FALSE(event.user_id.has_value());
}

TEST_F(ValidationTest, ErrorSeverityLevels) {
    // Тестируем все уровни Severity enum
    std::vector<Severity> severities = {Severity::Warning, Severity::Error, Severity::Critical};

    for (const auto& sev : severities) {
        ErrorEvent event;
        event.page = "/test";
        event.error_type = "TestError";
        event.message = "Test message";
        event.severity = sev;
        event.timestamp = 1234567890;

        EXPECT_EQ(event.severity, sev);
    }
}

TEST_F(ValidationTest, PerformancePositiveValues) {
    PerformanceEvent event;
    event.page = "/test";
    event.ttfb_ms = 100.0;
    event.total_page_load_ms = 1000.0;

    ASSERT_TRUE(event.ttfb_ms.has_value());
    EXPECT_GT(event.ttfb_ms.value(), 0);
    ASSERT_TRUE(event.total_page_load_ms.has_value());
    EXPECT_GT(event.total_page_load_ms.value(), 0);
}

// ===== Тесты граничных условий =====

class BoundaryConditionsTest : public ::testing::Test {};

TEST_F(BoundaryConditionsTest, VeryLongStrings) {
    PageViewEvent event;
    event.page = std::string(1000, 'a');
    event.user_id = std::string(500, 'b');

    EXPECT_EQ(event.page.length(), 1000);
    ASSERT_TRUE(event.user_id.has_value());
    EXPECT_EQ(event.user_id.value().length(), 500);
}

TEST_F(BoundaryConditionsTest, EmptyStrings) {
    PageViewEvent event;
    event.page = "";
    event.user_id = "";

    EXPECT_TRUE(event.page.empty());
    ASSERT_TRUE(event.user_id.has_value());
    EXPECT_TRUE(event.user_id.value().empty());
}

TEST_F(BoundaryConditionsTest, SpecialCharacters) {
    PageViewEvent event;
    event.page = "/products?id=123&sort=price";
    event.referrer = "https://example.com/path?query=test";

    EXPECT_NE(event.page.find("?"), std::string::npos);
    EXPECT_NE(event.page.find("&"), std::string::npos);
}

TEST_F(BoundaryConditionsTest, VeryLargePerformanceMetrics) {
    PerformanceEvent event;
    event.page = "/slow";
    event.total_page_load_ms = 999999.99;

    ASSERT_TRUE(event.total_page_load_ms.has_value());
    EXPECT_GT(event.total_page_load_ms.value(), 999999.0);
}

TEST_F(BoundaryConditionsTest, VerySmallPerformanceMetrics) {
    PerformanceEvent event;
    event.page = "/fast";
    event.ttfb_ms = 0.001;

    ASSERT_TRUE(event.ttfb_ms.has_value());
    EXPECT_LT(event.ttfb_ms.value(), 0.01);
}

// ===== Тесты JSON сериализации/десериализации =====

class JsonSerializationTest : public ::testing::Test {};

TEST_F(JsonSerializationTest, PageViewRoundTrip) {
    PageViewEvent original;
    original.page = "/test";
    original.user_id = "user-123";
    original.session_id = "sess-456";
    original.timestamp = 1234567890;

    nlohmann::json json = original;
    PageViewEvent deserialized = json.get<PageViewEvent>();

    EXPECT_EQ(original.page, deserialized.page);
    EXPECT_EQ(original.user_id, deserialized.user_id);
    EXPECT_EQ(original.session_id, deserialized.session_id);
}

TEST_F(JsonSerializationTest, PerformanceEventRoundTrip) {
    PerformanceEvent original;
    original.page = "/perf";
    original.ttfb_ms = 123.45;
    original.fcp_ms = 456.78;
    original.timestamp = 1234567890;

    nlohmann::json json = original;
    PerformanceEvent deserialized = json.get<PerformanceEvent>();

    EXPECT_EQ(original.page, deserialized.page);
    EXPECT_EQ(original.ttfb_ms, deserialized.ttfb_ms);
    EXPECT_EQ(original.fcp_ms, deserialized.fcp_ms);
}

TEST_F(JsonSerializationTest, ErrorEventRoundTrip) {
    ErrorEvent original;
    original.page = "/error";
    original.error_type = "TestError";
    original.message = "Test error message";
    original.severity = Severity::Critical;
    original.timestamp = 1234567890;

    nlohmann::json json = original;
    ErrorEvent deserialized = json.get<ErrorEvent>();

    EXPECT_EQ(original.page, deserialized.page);
    EXPECT_EQ(original.error_type, deserialized.error_type);
    EXPECT_EQ(original.severity, deserialized.severity);
}

