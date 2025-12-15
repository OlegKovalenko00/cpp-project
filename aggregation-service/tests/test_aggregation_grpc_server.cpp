#include <iostream>
#include <grpcpp/grpcpp.h>
#include "aggregation.grpc.pb.h"
#include <chrono>

using metricsys::aggregation::AggregationService;
using metricsys::aggregation::GetWatermarkRequest;
using metricsys::aggregation::GetWatermarkResponse;
using metricsys::aggregation::GetPageViewsAggRequest;
using metricsys::aggregation::GetPageViewsAggResponse;
using metricsys::aggregation::TimeRange;

int main(int argc, char** argv) {
    std::string server_address = "localhost:50052";

    if (argc > 1) {
        server_address = argv[1];
    }

    std::cout << "=== Testing Aggregation gRPC Server ===" << std::endl;
    std::cout << "Connecting to: " << server_address << std::endl << std::endl;

    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    auto stub = AggregationService::NewStub(channel);

    grpc::ClientContext context1;
    GetWatermarkRequest watermarkReq;
    GetWatermarkResponse watermarkResp;

    std::cout << "1. Testing GetWatermark()..." << std::endl;
    auto status1 = stub->GetWatermark(&context1, watermarkReq, &watermarkResp);

    if (status1.ok()) {
        std::cout << "   ✓ Watermark retrieved successfully" << std::endl;
        std::cout << "   Last aggregated at: " << watermarkResp.last_aggregated_at().seconds()
                  << " seconds since epoch" << std::endl;
    } else {
        std::cout << "   ✗ Error: " << status1.error_message() << std::endl;
    }
    std::cout << std::endl;

    // Тест GetPageViewsAgg
    grpc::ClientContext context2;
    GetPageViewsAggRequest pageViewsReq;
    GetPageViewsAggResponse pageViewsResp;

    pageViewsReq.set_project_id("test-project");

    // Задаем диапазон времени: последние 24 часа
    auto now = std::chrono::system_clock::now();
    auto yesterday = now - std::chrono::hours(24);

    auto* time_range = pageViewsReq.mutable_time_range();
    time_range->mutable_from()->set_seconds(
        std::chrono::duration_cast<std::chrono::seconds>(yesterday.time_since_epoch()).count()
    );
    time_range->mutable_to()->set_seconds(
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count()
    );

    // Pagination
    pageViewsReq.mutable_pagination()->set_limit(10);
    pageViewsReq.mutable_pagination()->set_offset(0);

    std::cout << "2. Testing GetPageViewsAgg()..." << std::endl;
    std::cout << "   Project ID: test-project" << std::endl;
    std::cout << "   Time range: last 24 hours" << std::endl;

    auto status2 = stub->GetPageViewsAgg(&context2, pageViewsReq, &pageViewsResp);

    if (status2.ok()) {
        std::cout << "   ✓ Page views retrieved successfully" << std::endl;
        std::cout << "   Found " << pageViewsResp.rows_size() << " aggregated page view records" << std::endl;

        if (pageViewsResp.rows_size() > 0) {
            std::cout << std::endl << "   Sample records:" << std::endl;
            for (int i = 0; i < std::min(3, pageViewsResp.rows_size()); ++i) {
                const auto& row = pageViewsResp.rows(i);
                std::cout << "   - Page: " << row.page()
                          << ", Views: " << row.views_count()
                          << ", Unique users: " << row.unique_users() << std::endl;
            }
        }
    } else {
        std::cout << "   ✗ Error: " << status2.error_message() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "=== Test Complete ===" << std::endl;

    return 0;
}

