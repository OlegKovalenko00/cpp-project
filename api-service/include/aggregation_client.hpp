#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

#include "aggregation.grpc.pb.h"

class AggregationClient {
public:
    explicit AggregationClient(std::shared_ptr<grpc::Channel> ch);

    grpc::Status GetPageViewsAgg(
        const metricsys::aggregation::GetPageViewsAggRequest& req,
        metricsys::aggregation::GetPageViewsAggResponse* resp,
        std::chrono::milliseconds timeout);
    
    grpc::Status GetClicksAgg(
        const m
    )
    
private:
    std::unique_ptr<metricsys::aggregation::AggregationService::Stub> stub_;
};
