#ifndef METRICS_CLIENT_H
#define METRICS_CLIENT_H

#include "aggregator.h"

#include <string>
#include <vector>
#include <chrono>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "metrics.grpc.pb.h"

namespace aggregation {

    class MetricsClient {
    public:
        MetricsClient(const std::string& host, const std::string& port);
        ~MetricsClient();

        // Получить все события всех типов за период
        std::vector<RawEvent> fetchAllEvents(
            std::chrono::system_clock::time_point from,
            std::chrono::system_clock::time_point to
        );

        // Отдельные методы для каждого типа событий
        std::vector<RawEvent> fetchPageViews(
            std::chrono::system_clock::time_point from,
            std::chrono::system_clock::time_point to
        );

        std::vector<RawEvent> fetchClicks(
            std::chrono::system_clock::time_point from,
            std::chrono::system_clock::time_point to
        );

        std::vector<RawEvent> fetchPerformance(
            std::chrono::system_clock::time_point from,
            std::chrono::system_clock::time_point to
        );

        std::vector<RawEvent> fetchErrors(
            std::chrono::system_clock::time_point from,
            std::chrono::system_clock::time_point to
        );

        std::vector<RawEvent> fetchCustomEvents(
            std::chrono::system_clock::time_point from,
            std::chrono::system_clock::time_point to
        );

        bool isConnected() const;

    private:
        metricsys::TimeRange makeTimeRange(
            std::chrono::system_clock::time_point from,
            std::chrono::system_clock::time_point to
        ) const;

        std::chrono::system_clock::time_point timestampToTimePoint(int64_t ts) const;

        std::shared_ptr<grpc::Channel> channel_;
        std::unique_ptr<metricsys::MetricsService::Stub> stub_;
        std::string projectId_;
    };

} // namespace aggregation

#endif // METRICS_CLIENT_H

