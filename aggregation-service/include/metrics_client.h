#ifndef METRICS_CLIENT_H
#define METRICS_CLIENT_H

#include "aggregator.h"

#include <string>
#include <vector>
#include <chrono>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "metrics_service.grpc.pb.h"

namespace aggregation {

    class MetricsClient {
    public:
        MetricsClient(const std::string& host, const std::string& port);
        ~MetricsClient();

        std::vector<RawEvent> fetchRawEvents(
            const std::chrono::system_clock::time_point& startTime,
            const std::chrono::system_clock::time_point& endTime
        );

        bool isConnected() const;

    private:
        std::string formatTimestamp(std::chrono::system_clock::time_point tp) const;
        std::chrono::system_clock::time_point parseTimestamp(const std::string& timestampStr) const;

        std::shared_ptr<grpc::Channel> channel_;
        std::unique_ptr<metrics::MetricsService::Stub> stub_;
    };

} // namespace aggregation

#endif // METRICS_CLIENT_H
