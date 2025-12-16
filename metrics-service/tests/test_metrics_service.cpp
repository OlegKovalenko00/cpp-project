#include "metrics.h"

#include <gtest/gtest.h>

#include "database.h"
#include "test_utils.hpp"

class MetricsServiceTest : public ::testing::Test {
protected:
    DatabaseConfig config = TestDatabaseConfig();

    void SetUp() override {
        ResetDatabase(config);
    }

    MetricsServiceImpl makeService() {
        return MetricsServiceImpl(config);
    }
};

TEST_F(MetricsServiceTest, GetPageViewsReturnsPersistedEvents) {
    PageView event;
    event.page = "/grpc-page";
    event.user_id = "grpc-user";
    save_page_view(config, event);

    MetricsServiceImpl service = makeService();
    grpc::ServerContext ctx;
    metricsys::GetPageViewsRequest request;
    request.set_page_filter("grpc-page");
    metricsys::GetPageViewsResponse response;

    auto status = service.GetPageViews(&ctx, &request, &response);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(response.events_size(), 1);
    EXPECT_EQ(response.events(0).page(), event.page);
}

TEST_F(MetricsServiceTest, GetClicksSupportsFilters) {
    ClickEvent event;
    event.page = "/grpc-click";
    event.element_id = "grpc-element";
    save_click_event(config, event);

    MetricsServiceImpl service = makeService();
    grpc::ServerContext ctx;
    metricsys::GetClicksRequest request;
    request.set_element_id_filter("grpc-element");
    metricsys::GetClicksResponse response;

    auto status = service.GetClicks(&ctx, &request, &response);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(response.events_size(), 1);
    EXPECT_EQ(response.events(0).element_id(), *event.element_id);
}

TEST_F(MetricsServiceTest, GetPerformanceReturnsTimingData) {
    PerformanceEvent event;
    event.page = "/grpc-perf";
    event.ttfb_ms = 123.4;
    event.user_id = "grpc-user";
    save_performance_event(config, event);

    MetricsServiceImpl service = makeService();
    grpc::ServerContext ctx;
    metricsys::GetPerformanceRequest request;
    request.set_page_filter("grpc-perf");
    metricsys::GetPerformanceResponse response;

    auto status = service.GetPerformance(&ctx, &request, &response);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(response.events_size(), 1);
    EXPECT_DOUBLE_EQ(response.events(0).ttfb_ms(), *event.ttfb_ms);
}

TEST_F(MetricsServiceTest, GetErrorsRespectsSeverityFilter) {
    ErrorEvent event;
    event.page = "/grpc-error";
    event.error_type = "TypeError";
    event.message = "boom";
    event.severity = 3;
    save_error_event(config, event);

    MetricsServiceImpl service = makeService();
    grpc::ServerContext ctx;
    metricsys::GetErrorsRequest request;
    request.set_severity_filter(metricsys::SEVERITY_ERROR);
    metricsys::GetErrorsResponse response;

    auto status = service.GetErrors(&ctx, &request, &response);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(response.events_size(), 1);
    EXPECT_EQ(response.events(0).severity(), metricsys::SEVERITY_CRITICAL);
}

TEST_F(MetricsServiceTest, GetCustomEventsFiltersByName) {
    CustomEvent event;
    event.name = "grpc-event";
    event.page = "/grpc-custom";
    save_custom_event(config, event);

    MetricsServiceImpl service = makeService();
    grpc::ServerContext ctx;
    metricsys::GetCustomEventsRequest request;
    request.set_name_filter("grpc-event");
    metricsys::GetCustomEventsResponse response;

    auto status = service.GetCustomEvents(&ctx, &request, &response);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(response.events_size(), 1);
    EXPECT_EQ(response.events(0).name(), event.name);
}
