#include "aggregation_client.hpp"

AggregationClient::AggregationClient(std::shared_ptr<grpc::Channel> ch)
        : stub_(metricsys::aggregation::AggregationService::NewStub(std::move(ch))) {}


grpc::Status AggregationClient::GetWatermark(
        const metricsys::aggregation::GetWatermarkRequest& req,
        metricsys::aggregation::GetWatermarkResponse* resp,
        std::chrono::milliseconds timeout)
    {
        grpc::ClientContext ctx;
        ctx.set_deadline(std::chrono::system_clock::now() + timeout);
        return stub_->GetWatermark(&ctx, req, resp);
    }


 grpc::Status AggregationClient::GetPageViewsAgg(
        const metricsys::aggregation::GetPageViewsAggRequest& req,
        metricsys::aggregation::GetPageViewsAggResponse* resp,
        std::chrono::milliseconds timeout)
    {
        grpc::ClientContext ctx;
        ctx.set_deadline(std::chrono::system_clock::now() + timeout);
        return stub_->GetPageViewsAgg(&ctx, req, resp);
    }

grpc::Status AggregationClient::GetClicksAgg(
        const metricsys::aggregation::GetClicksAggRequest& req,
        metricsys::aggregation::GetClicksAggResponse* resp,
        std::chrono::milliseconds timeout
    ) {
        grpc::ClientContext ctx;
        ctx.set_deadline(std::chrono::system_clock::now() + timeout);
        return stub_->GetClicksAgg(&ctx, req, resp);
    }

grpc::Status AggregationClient::GetPerformanceAgg(
        const metricsys::aggregation::GetPerformanceAggRequest& req,
        metricsys::aggregation::GetPerformanceAggResponse* resp,
        std::chrono::milliseconds timeout
    ) {
        grpc::ClientContext ctx;
        ctx.set_deadline(std::chrono::system_clock::now() + timeout);
        return stub_->GetPerformanceAgg(&ctx, req, resp);
    }

grpc::Status AggregationClient::GetErrorsAgg(
        const metricsys::aggregation::GetErrorsAggRequest& req,
        metricsys::aggregation::GetErrorsAggResponse* resp,
        std::chrono::milliseconds timeout
    ) {
        grpc::ClientContext ctx;
        ctx.set_deadline(std::chrono::system_clock::now() + timeout);
        return stub_->GetErrorsAgg(&ctx, req, resp);
    }

grpc::Status AggregationClient::GetCustomEventsAgg(
        const metricsys::aggregation::GetCustomEventsAggRequest& req,
        metricsys::aggregation::GetCustomEventsAggResponse* resp,
        std::chrono::milliseconds timeout
    ) {
        grpc::ClientContext ctx;
        ctx.set_deadline(std::chrono::system_clock::now() + timeout);
        return stub_->GetCustomEventsAgg(&ctx, req, resp);
    }