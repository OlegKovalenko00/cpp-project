#include "metrics_client.h"

#include <iostream>

namespace aggregation {

MetricsClient::MetricsClient(const std::string& host, const std::string& port)
    : projectId_("default-project")
{
    std::string target = host + ":" + port;
    channel_ = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub_ = metricsys::MetricsService::NewStub(channel_);
    std::cout << "MetricsClient: connecting to " << target << std::endl;
}

MetricsClient::~MetricsClient() = default;

bool MetricsClient::isConnected() const {
    if (!channel_) {
        return false;
    }
    auto state = channel_->GetState(false);
    return state == GRPC_CHANNEL_READY || state == GRPC_CHANNEL_IDLE;
}

metricsys::TimeRange MetricsClient::makeTimeRange(
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) const {
    metricsys::TimeRange range;
    auto fromMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        from.time_since_epoch()).count();
    auto toMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        to.time_since_epoch()).count();
    range.set_start_timestamp(fromMs);
    range.set_end_timestamp(toMs);
    return range;
}

std::chrono::system_clock::time_point MetricsClient::timestampToTimePoint(int64_t ts) const {
    return std::chrono::system_clock::time_point(
        std::chrono::milliseconds(ts)
    );
}

std::vector<RawEvent> MetricsClient::fetchAllEvents(
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) {
    std::vector<RawEvent> allEvents;

    auto pageViews = fetchPageViews(from, to);
    allEvents.insert(allEvents.end(), pageViews.begin(), pageViews.end());

    auto clicks = fetchClicks(from, to);
    allEvents.insert(allEvents.end(), clicks.begin(), clicks.end());

    auto performance = fetchPerformance(from, to);
    allEvents.insert(allEvents.end(), performance.begin(), performance.end());

    auto errors = fetchErrors(from, to);
    allEvents.insert(allEvents.end(), errors.begin(), errors.end());

    auto custom = fetchCustomEvents(from, to);
    allEvents.insert(allEvents.end(), custom.begin(), custom.end());

    std::cout << "MetricsClient: fetched " << allEvents.size() << " total events" << std::endl;
    return allEvents;
}

std::vector<RawEvent> MetricsClient::fetchPageViews(
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) {
    std::vector<RawEvent> result;

    metricsys::GetPageViewsRequest request;
    *request.mutable_time_range() = makeTimeRange(from, to);

    metricsys::GetPageViewsResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->GetPageViews(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "MetricsClient: GetPageViews error - "
                  << status.error_code() << ": "
                  << status.error_message() << std::endl;
        return result;
    }

    for (const auto& event : response.events()) {
        RawEvent raw;
        raw.projectId = projectId_;
        raw.page = event.page();
        raw.eventType = "page_view";
        raw.isError = false;
        raw.userId = event.has_user_id() ? event.user_id() : "";
        raw.sessionId = event.has_session_id() ? event.session_id() : "";
        raw.timestamp = timestampToTimePoint(event.timestamp());
        result.push_back(raw);
    }

    std::cout << "MetricsClient: received " << result.size() << " page_view events" << std::endl;
    return result;
}

std::vector<RawEvent> MetricsClient::fetchClicks(
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) {
    std::vector<RawEvent> result;

    metricsys::GetClicksRequest request;
    *request.mutable_time_range() = makeTimeRange(from, to);

    metricsys::GetClicksResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->GetClicks(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "MetricsClient: GetClicks error - "
                  << status.error_code() << ": "
                  << status.error_message() << std::endl;
        return result;
    }

    for (const auto& event : response.events()) {
        RawEvent raw;
        raw.projectId = projectId_;
        raw.page = event.page();
        raw.eventType = "click";
        raw.isError = false;
        raw.userId = event.has_user_id() ? event.user_id() : "";
        raw.sessionId = event.has_session_id() ? event.session_id() : "";
        raw.timestamp = timestampToTimePoint(event.timestamp());
        raw.elementId = event.element_id();
        result.push_back(raw);
    }

    std::cout << "MetricsClient: received " << result.size() << " click events" << std::endl;
    return result;
}

std::vector<RawEvent> MetricsClient::fetchPerformance(
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) {
    std::vector<RawEvent> result;

    metricsys::GetPerformanceRequest request;
    *request.mutable_time_range() = makeTimeRange(from, to);

    metricsys::GetPerformanceResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->GetPerformance(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "MetricsClient: GetPerformance error - "
                  << status.error_code() << ": "
                  << status.error_message() << std::endl;
        return result;
    }

    for (const auto& event : response.events()) {
        RawEvent raw;
        raw.projectId = projectId_;
        raw.page = event.page();
        raw.eventType = "performance";
        raw.isError = false;
        raw.userId = event.has_user_id() ? event.user_id() : "";
        raw.sessionId = event.has_session_id() ? event.session_id() : "";
        raw.timestamp = timestampToTimePoint(event.timestamp());

        // Performance metrics from proto
        raw.totalPageLoadMs = event.has_total_page_load_ms() ? event.total_page_load_ms() : 0.0;
        raw.ttfbMs = event.has_ttfb_ms() ? event.ttfb_ms() : 0.0;
        raw.fcpMs = event.has_fcp_ms() ? event.fcp_ms() : 0.0;
        raw.lcpMs = event.has_lcp_ms() ? event.lcp_ms() : 0.0;

        result.push_back(raw);
    }

    std::cout << "MetricsClient: received " << result.size() << " performance events" << std::endl;
    return result;
}

std::vector<RawEvent> MetricsClient::fetchErrors(
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) {
    std::vector<RawEvent> result;

    metricsys::GetErrorsRequest request;
    *request.mutable_time_range() = makeTimeRange(from, to);

    metricsys::GetErrorsResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->GetErrors(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "MetricsClient: GetErrors error - "
                  << status.error_code() << ": "
                  << status.error_message() << std::endl;
        return result;
    }

    for (const auto& event : response.events()) {
        RawEvent raw;
        raw.projectId = projectId_;
        raw.page = event.page();
        raw.eventType = "error";
        raw.isError = true;
        raw.userId = event.has_user_id() ? event.user_id() : "";
        raw.sessionId = event.has_session_id() ? event.session_id() : "";
        raw.timestamp = timestampToTimePoint(event.timestamp());
        raw.errorType = event.error_type();
        raw.errorMessage = event.message();
        raw.severity = static_cast<int>(event.severity());
        result.push_back(raw);
    }

    std::cout << "MetricsClient: received " << result.size() << " error events" << std::endl;
    return result;
}

std::vector<RawEvent> MetricsClient::fetchCustomEvents(
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) {
    std::vector<RawEvent> result;

    metricsys::GetCustomEventsRequest request;
    *request.mutable_time_range() = makeTimeRange(from, to);

    metricsys::GetCustomEventsResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->GetCustomEvents(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "MetricsClient: GetCustomEvents error - "
                  << status.error_code() << ": "
                  << status.error_message() << std::endl;
        return result;
    }

    for (const auto& event : response.events()) {
        RawEvent raw;
        raw.projectId = projectId_;
        raw.page = event.has_page() ? event.page() : "";
        raw.eventType = "custom";
        raw.isError = false;
        raw.userId = event.has_user_id() ? event.user_id() : "";
        raw.sessionId = event.has_session_id() ? event.session_id() : "";
        raw.timestamp = timestampToTimePoint(event.timestamp());
        raw.customEventName = event.name();
        for (const auto& [key, value] : event.properties()) {
            raw.properties[key] = value;
        }
        result.push_back(raw);
    }

    std::cout << "MetricsClient: received " << result.size() << " custom events" << std::endl;
    return result;
}

} // namespace aggregation
