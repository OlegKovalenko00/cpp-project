#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

#include "aggregation.grpc.pb.h"


/*
 rpc GetWatermark(GetWatermarkRequest) returns (GetWatermarkResponse);
  rpc GetPageViewsAgg(GetPageViewsAggRequest) returns (GetPageViewsAggResponse);
  rpc GetClicksAgg(GetClicksAggRequest) returns (GetClicksAggResponse);
  rpc GetPerformanceAgg(GetPerformanceAggRequest) returns (GetPerformanceAggResponse);
  rpc GetErrorsAgg(GetErrorsAggRequest) returns (GetErrorsAggResponse);
  rpc GetCustomEventsAgg(GetCustomEventsAggRequest) returns (GetCustomEventsAggResponse);
*/

class AggregationClient {
public:
    explicit AggregationClient(std::shared_ptr<grpc::Channel> ch);

    grpc::Status GetWatermark(
        const metricsys::aggregation::GetWatermarkRequest& req,
        metricsys::aggregation::GetWatermarkResponse* resp,
        std::chrono::milliseconds timeout
    );

    grpc::Status GetPageViewsAgg(
        const metricsys::aggregation::GetPageViewsAggRequest& req,
        metricsys::aggregation::GetPageViewsAggResponse* resp,
        std::chrono::milliseconds timeout);
    
    grpc::Status GetClicksAgg(
        const metricsys::aggregation::GetClicksAggRequest& req,
        metricsys::aggregation::GetClicksAggResponse* resp,
        std::chrono::milliseconds timeout
    );

    grpc::Status GetPerformanceAgg(
        const metricsys::aggregation::GetPerformanceAggRequest& req,
        metricsys::aggregation::GetPerformanceAggResponse* resp,
        std::chrono::milliseconds timeout
    );

    grpc::Status GetErrorsAgg(
        const metricsys::aggregation::GetErrorsAggRequest& req,
        metricsys::aggregation::GetErrorsAggResponse* resp,
        std::chrono::milliseconds timeout
    );

    grpc::Status GetCustomEventsAgg(
        const metricsys::aggregation::GetCustomEventsAggRequest& req,
        metricsys::aggregation::GetCustomEventsAggResponse* resp,
        std::chrono::milliseconds timeout
    );
    
private:
    std::unique_ptr<metricsys::aggregation::AggregationService::Stub> stub_;
};
