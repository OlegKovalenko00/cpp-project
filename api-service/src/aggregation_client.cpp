#include "aggregation_client.hpp"

AAggregationClient::AggregationClient(std::shared_ptr<grpc::Channel> ch)
        : stub_(metricsys::aggregation::AggregationService::NewStub(std::move(ch))) {}

 grpc::Status AggregationClient::GetPageViewsAgg(
        const metricsys::aggregation::GetPageViewsAggRequest& req,
        metricsys::aggregation::GetPageViewsAggResponse* resp,
        std::chrono::milliseconds timeout)
    {
        grpc::ClientContext ctx;
        ctx.set_deadline(std::chrono::system_clock::now() + timeout);
        return stub_->GetPageViewsAgg(&ctx, req, resp);
    }