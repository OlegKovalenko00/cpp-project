#ifndef AGGREGATION_SERVER_H
#define AGGREGATION_SERVER_H

#include <grpcpp/grpcpp.h>
#include "aggregation.grpc.pb.h"
#include <memory>
#include <string>

namespace aggregation {

class Database;

// gRPC сервер для предоставления агрегированных данных
class AggregationServiceImpl final : public metricsys::aggregation::AggregationService::Service {
public:
    explicit AggregationServiceImpl(Database& db);
    ~AggregationServiceImpl() override;

    // Получить watermark
    grpc::Status GetWatermark(
        grpc::ServerContext* context,
        const metricsys::aggregation::GetWatermarkRequest* request,
        metricsys::aggregation::GetWatermarkResponse* response
    ) override;

    // Получить агрегированные page views
    grpc::Status GetPageViewsAgg(
        grpc::ServerContext* context,
        const metricsys::aggregation::GetPageViewsAggRequest* request,
        metricsys::aggregation::GetPageViewsAggResponse* response
    ) override;

    // Получить агрегированные clicks
    grpc::Status GetClicksAgg(
        grpc::ServerContext* context,
        const metricsys::aggregation::GetClicksAggRequest* request,
        metricsys::aggregation::GetClicksAggResponse* response
    ) override;

    // Получить агрегированные performance метрики
    grpc::Status GetPerformanceAgg(
        grpc::ServerContext* context,
        const metricsys::aggregation::GetPerformanceAggRequest* request,
        metricsys::aggregation::GetPerformanceAggResponse* response
    ) override;

    // Получить агрегированные errors
    grpc::Status GetErrorsAgg(
        grpc::ServerContext* context,
        const metricsys::aggregation::GetErrorsAggRequest* request,
        metricsys::aggregation::GetErrorsAggResponse* response
    ) override;

    // Получить агрегированные custom events
    grpc::Status GetCustomEventsAgg(
        grpc::ServerContext* context,
        const metricsys::aggregation::GetCustomEventsAggRequest* request,
        metricsys::aggregation::GetCustomEventsAggResponse* response
    ) override;

private:
    Database& database_;
};

// Запуск gRPC сервера в отдельном потоке
class AggregationGrpcServer {
public:
    AggregationGrpcServer(Database& db, const std::string& server_address);
    ~AggregationGrpcServer();

    void start();
    void stop();
    void wait();

private:
    std::unique_ptr<AggregationServiceImpl> service_;
    std::unique_ptr<grpc::Server> server_;
    std::string server_address_;
};

} // namespace aggregation

#endif // AGGREGATION_SERVER_H

