#include "aggregation_server.h"
#include "database.h"
#include "aggregator.h"
#include <iostream>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

namespace aggregation {

// Вспомогательная функция конвертации chrono в protobuf timestamp
static void timePointToTimestamp(
    std::chrono::system_clock::time_point tp,
    google::protobuf::Timestamp* timestamp
) {
    auto duration = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);

    timestamp->set_seconds(seconds.count());
    timestamp->set_nanos(nanos.count());
}

// Вспомогательная функция конвертации protobuf timestamp в chrono
static std::chrono::system_clock::time_point timestampToTimePoint(
    const google::protobuf::Timestamp& timestamp
) {
    auto seconds = std::chrono::seconds(timestamp.seconds());
    auto nanos = std::chrono::nanoseconds(timestamp.nanos());
    return std::chrono::system_clock::time_point(seconds + nanos);
}

// ===== AggregationServiceImpl =====

AggregationServiceImpl::AggregationServiceImpl(Database& db)
    : database_(db) {
}

AggregationServiceImpl::~AggregationServiceImpl() = default;

grpc::Status AggregationServiceImpl::GetWatermark(
    grpc::ServerContext* context,
    const metricsys::aggregation::GetWatermarkRequest* request,
    metricsys::aggregation::GetWatermarkResponse* response
) {
    try {
        auto watermark = database_.getWatermark();
        timePointToTimestamp(watermark, response->mutable_last_aggregated_at());
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AggregationServiceImpl::GetPageViewsAgg(
    grpc::ServerContext* context,
    const metricsys::aggregation::GetPageViewsAggRequest* request,
    metricsys::aggregation::GetPageViewsAggResponse* response
) {
    try {
        if (request->project_id().empty()) {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "project_id is required");
        }

        auto from = timestampToTimePoint(request->time_range().from());
        auto to = timestampToTimePoint(request->time_range().to());

        std::string pageFilter = request->has_page() ? request->page() : "";
        int limit = request->pagination().limit() > 0 ? request->pagination().limit() : 1000;
        int offset = request->pagination().offset();

        auto data = database_.readPageViews(
            request->project_id(),
            from,
            to,
            pageFilter,
            limit,
            offset
        );

        for (const auto& item : data) {
            auto* row = response->add_rows();
            timePointToTimestamp(item.timeBucket, row->mutable_time_bucket());
            row->set_project_id(item.projectId);
            row->set_page(item.page);
            row->set_views_count(item.viewsCount);
            row->set_unique_users(item.uniqueUsers);
            row->set_unique_sessions(item.uniqueSessions);
            timePointToTimestamp(std::chrono::system_clock::now(), row->mutable_created_at());
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AggregationServiceImpl::GetClicksAgg(
    grpc::ServerContext* context,
    const metricsys::aggregation::GetClicksAggRequest* request,
    metricsys::aggregation::GetClicksAggResponse* response
) {
    try {
        if (request->project_id().empty()) {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "project_id is required");
        }

        auto from = timestampToTimePoint(request->time_range().from());
        auto to = timestampToTimePoint(request->time_range().to());

        std::string pageFilter = request->has_page() ? request->page() : "";
        std::string elementIdFilter = request->has_element_id() ? request->element_id() : "";
        int limit = request->pagination().limit() > 0 ? request->pagination().limit() : 1000;
        int offset = request->pagination().offset();

        auto data = database_.readClicks(
            request->project_id(),
            from,
            to,
            pageFilter,
            elementIdFilter,
            limit,
            offset
        );

        for (const auto& item : data) {
            auto* row = response->add_rows();
            timePointToTimestamp(item.timeBucket, row->mutable_time_bucket());
            row->set_project_id(item.projectId);
            row->set_page(item.page);
            if (!item.elementId.empty()) {
                row->set_element_id(item.elementId);
            }
            row->set_clicks_count(item.clicksCount);
            row->set_unique_users(item.uniqueUsers);
            row->set_unique_sessions(item.uniqueSessions);
            timePointToTimestamp(std::chrono::system_clock::now(), row->mutable_created_at());
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AggregationServiceImpl::GetPerformanceAgg(
    grpc::ServerContext* context,
    const metricsys::aggregation::GetPerformanceAggRequest* request,
    metricsys::aggregation::GetPerformanceAggResponse* response
) {
    try {
        if (request->project_id().empty()) {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "project_id is required");
        }

        auto from = timestampToTimePoint(request->time_range().from());
        auto to = timestampToTimePoint(request->time_range().to());

        std::string pageFilter = request->has_page() ? request->page() : "";
        int limit = request->pagination().limit() > 0 ? request->pagination().limit() : 1000;
        int offset = request->pagination().offset();

        auto data = database_.readPerformance(
            request->project_id(),
            from,
            to,
            pageFilter,
            limit,
            offset
        );

        for (const auto& item : data) {
            auto* row = response->add_rows();
            timePointToTimestamp(item.timeBucket, row->mutable_time_bucket());
            row->set_project_id(item.projectId);
            row->set_page(item.page);
            row->set_samples_count(item.samplesCount);
            row->set_avg_total_load_ms(item.avgTotalLoadMs);
            row->set_p95_total_load_ms(item.p95TotalLoadMs);
            row->set_avg_ttfb_ms(item.avgTtfbMs);
            row->set_p95_ttfb_ms(item.p95TtfbMs);
            row->set_avg_fcp_ms(item.avgFcpMs);
            row->set_p95_fcp_ms(item.p95FcpMs);
            row->set_avg_lcp_ms(item.avgLcpMs);
            row->set_p95_lcp_ms(item.p95LcpMs);
            timePointToTimestamp(std::chrono::system_clock::now(), row->mutable_created_at());
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AggregationServiceImpl::GetErrorsAgg(
    grpc::ServerContext* context,
    const metricsys::aggregation::GetErrorsAggRequest* request,
    metricsys::aggregation::GetErrorsAggResponse* response
) {
    try {
        if (request->project_id().empty()) {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "project_id is required");
        }

        auto from = timestampToTimePoint(request->time_range().from());
        auto to = timestampToTimePoint(request->time_range().to());

        std::string pageFilter = request->has_page() ? request->page() : "";
        std::string errorTypeFilter = request->has_error_type() ? request->error_type() : "";
        int limit = request->pagination().limit() > 0 ? request->pagination().limit() : 1000;
        int offset = request->pagination().offset();

        auto data = database_.readErrors(
            request->project_id(),
            from,
            to,
            pageFilter,
            errorTypeFilter,
            limit,
            offset
        );

        for (const auto& item : data) {
            auto* row = response->add_rows();
            timePointToTimestamp(item.timeBucket, row->mutable_time_bucket());
            row->set_project_id(item.projectId);
            row->set_page(item.page);
            if (!item.errorType.empty()) {
                row->set_error_type(item.errorType);
            }
            row->set_errors_count(item.errorsCount);
            row->set_warning_count(item.warningCount);
            row->set_critical_count(item.criticalCount);
            row->set_unique_users(item.uniqueUsers);
            timePointToTimestamp(std::chrono::system_clock::now(), row->mutable_created_at());
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AggregationServiceImpl::GetCustomEventsAgg(
    grpc::ServerContext* context,
    const metricsys::aggregation::GetCustomEventsAggRequest* request,
    metricsys::aggregation::GetCustomEventsAggResponse* response
) {
    try {
        if (request->project_id().empty()) {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "project_id is required");
        }
        if (request->event_name().empty()) {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "event_name is required");
        }

        auto from = timestampToTimePoint(request->time_range().from());
        auto to = timestampToTimePoint(request->time_range().to());

        std::string pageFilter = request->has_page() ? request->page() : "";
        int limit = request->pagination().limit() > 0 ? request->pagination().limit() : 1000;
        int offset = request->pagination().offset();

        auto data = database_.readCustomEvents(
            request->project_id(),
            from,
            to,
            request->event_name(),
            pageFilter,
            limit,
            offset
        );

        for (const auto& item : data) {
            auto* row = response->add_rows();
            timePointToTimestamp(item.timeBucket, row->mutable_time_bucket());
            row->set_project_id(item.projectId);
            row->set_event_name(item.eventName);
            if (!item.page.empty()) {
                row->set_page(item.page);
            }
            row->set_events_count(item.eventsCount);
            row->set_unique_users(item.uniqueUsers);
            row->set_unique_sessions(item.uniqueSessions);
            timePointToTimestamp(std::chrono::system_clock::now(), row->mutable_created_at());
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

// ===== AggregationGrpcServer =====

AggregationGrpcServer::AggregationGrpcServer(Database& db, const std::string& server_address)
    : service_(std::make_unique<AggregationServiceImpl>(db))
    , server_address_(server_address) {
}

AggregationGrpcServer::~AggregationGrpcServer() {
    stop();
}

void AggregationGrpcServer::start() {
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());
    builder.RegisterService(service_.get());

    server_ = builder.BuildAndStart();

    if (server_) {
        std::cout << "[gRPC] Aggregation gRPC server listening on " << server_address_ << std::endl;
    } else {
        std::cerr << "[gRPC] Failed to start Aggregation gRPC server" << std::endl;
    }
}

void AggregationGrpcServer::stop() {
    if (server_) {
        std::cout << "[gRPC] Stopping Aggregation gRPC server..." << std::endl;
        server_->Shutdown();
        server_.reset();
    }
}

void AggregationGrpcServer::wait() {
    if (server_) {
        server_->Wait();
    }
}

} // namespace aggregation

