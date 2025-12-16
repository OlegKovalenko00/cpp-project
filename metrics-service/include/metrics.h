#pragma once

#include "metrics.grpc.pb.h"
#include "metrics.pb.h"
#include "database.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

class MetricsServiceImpl final : public metricsys::MetricsService::Service {
public:
    explicit MetricsServiceImpl(const DatabaseConfig& db_config);

    grpc::Status GetPageViews(
        grpc::ServerContext* context,
        const metricsys::GetPageViewsRequest* request,
        metricsys::GetPageViewsResponse* response) override;

    grpc::Status GetClicks(
        grpc::ServerContext* context,
        const metricsys::GetClicksRequest* request,
        metricsys::GetClicksResponse* response) override;

    grpc::Status GetPerformance(
        grpc::ServerContext* context,
        const metricsys::GetPerformanceRequest* request,
        metricsys::GetPerformanceResponse* response) override;

    grpc::Status GetErrors(
        grpc::ServerContext* context,
        const metricsys::GetErrorsRequest* request,
        metricsys::GetErrorsResponse* response) override;

    grpc::Status GetCustomEvents(
        grpc::ServerContext* context,
        const metricsys::GetCustomEventsRequest* request,
        metricsys::GetCustomEventsResponse* response) override;

private:
    DatabaseConfig db_config_;
    std::string get_connection_string() const;
};

void run_grpc_server(const std::string& address, const DatabaseConfig& db_config);
