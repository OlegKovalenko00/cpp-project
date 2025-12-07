#include "metrics_client.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace aggregation {

MetricsClient::MetricsClient(const std::string& host, const std::string& port) {
    std::string target = host + ":" + port;
    channel_ = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub_ = metrics::MetricsService::NewStub(channel_);
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

std::vector<RawEvent> MetricsClient::fetchRawEvents(
    const std::chrono::system_clock::time_point& startTime,
    const std::chrono::system_clock::time_point& endTime
) {
    std::vector<RawEvent> result;

    metrics::ListEventsRequest request;
    request.set_from_timestamp(formatTimestamp(startTime));
    request.set_to_timestamp(formatTimestamp(endTime));

    metrics::ListEventsResponse response;
    grpc::ClientContext context;

    std::cout << "MetricsClient: fetching events from " 
              << request.from_timestamp() << " to " 
              << request.to_timestamp() << std::endl;

    grpc::Status status = stub_->ListEvents(&context, request, &response);

    if (!status.ok()) {
        std::cerr << "MetricsClient: gRPC error - " 
                  << status.error_code() << ": " 
                  << status.error_message() << std::endl;
        return result;
    }

    for (const auto& protoEvent : response.events()) {
        RawEvent event;
        event.projectId = protoEvent.project_id();
        event.page = protoEvent.page();
        event.eventType = protoEvent.event_type();
        event.performanceMs = protoEvent.performance_ms();
        event.isError = protoEvent.is_error();
        event.userId = protoEvent.user_id();
        event.sessionId = protoEvent.session_id();
        event.timestamp = parseTimestamp(protoEvent.timestamp());
        result.push_back(event);
    }

    std::cout << "MetricsClient: received " << result.size() << " events" << std::endl;
    return result;
}

std::string MetricsClient::formatTimestamp(std::chrono::system_clock::time_point tp) const {
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_val{};
    gmtime_r(&time_t_val, &tm_val);

    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::chrono::system_clock::time_point MetricsClient::parseTimestamp(const std::string& timestampStr) const {
    std::tm tm_val{};
    std::istringstream iss(timestampStr);
    iss >> std::get_time(&tm_val, "%Y-%m-%dT%H:%M:%SZ");

    if (iss.fail()) {
        std::cerr << "MetricsClient: failed to parse timestamp: " << timestampStr << std::endl;
        return std::chrono::system_clock::time_point{};
    }

    std::time_t time_t_val = timegm(&tm_val);
    return std::chrono::system_clock::from_time_t(time_t_val);
}

} // namespace aggregation
