// filepath: /home/arsenchik/programming/cppprojects/CPP_second_year/cpp-project/aggregation-service/tests/test_grpc_connection.cpp
#include <iostream>
#include <chrono>
#include "metrics_client.h"

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    std::string port = "50051";

    if (argc >= 2) {
        host = argv[1];
    }
    if (argc >= 3) {
        port = argv[2];
    }

    std::cout << "=== gRPC Connection Test ===" << std::endl;
    std::cout << "Connecting to metrics-service at " << host << ":" << port << std::endl;

    aggregation::MetricsClient client(host, port);

    std::cout << "Channel state: " << (client.isConnected() ? "READY/IDLE" : "NOT READY") << std::endl;

    // Попробуем получить события за последний час
    auto now = std::chrono::system_clock::now();
    auto oneHourAgo = now - std::chrono::hours(1);

    std::cout << "\nFetching page views..." << std::endl;
    auto pageViews = client.fetchPageViews(oneHourAgo, now);
    std::cout << "Received " << pageViews.size() << " page view events" << std::endl;

    std::cout << "\nFetching clicks..." << std::endl;
    auto clicks = client.fetchClicks(oneHourAgo, now);
    std::cout << "Received " << clicks.size() << " click events" << std::endl;

    std::cout << "\nFetching performance events..." << std::endl;
    auto perf = client.fetchPerformance(oneHourAgo, now);
    std::cout << "Received " << perf.size() << " performance events" << std::endl;

    std::cout << "\nFetching errors..." << std::endl;
    auto errors = client.fetchErrors(oneHourAgo, now);
    std::cout << "Received " << errors.size() << " error events" << std::endl;

    std::cout << "\nFetching custom events..." << std::endl;
    auto custom = client.fetchCustomEvents(oneHourAgo, now);
    std::cout << "Received " << custom.size() << " custom events" << std::endl;

    std::cout << "\n=== Test Complete ===" << std::endl;

    // Выводим подробности о полученных событиях
    if (!pageViews.empty()) {
        std::cout << "\nSample page view:" << std::endl;
        const auto& pv = pageViews[0];
        std::cout << "  Page: " << pv.page << std::endl;
        std::cout << "  User ID: " << pv.userId << std::endl;
        std::cout << "  Session ID: " << pv.sessionId << std::endl;
    }

    if (!perf.empty()) {
        std::cout << "\nSample performance event:" << std::endl;
        const auto& p = perf[0];
        std::cout << "  Page: " << p.page << std::endl;
        std::cout << "  Total Load: " << p.totalPageLoadMs << " ms" << std::endl;
        std::cout << "  TTFB: " << p.ttfbMs << " ms" << std::endl;
        std::cout << "  FCP: " << p.fcpMs << " ms" << std::endl;
        std::cout << "  LCP: " << p.lcpMs << " ms" << std::endl;
    }

    if (!errors.empty()) {
        std::cout << "\nSample error:" << std::endl;
        const auto& e = errors[0];
        std::cout << "  Page: " << e.page << std::endl;
        std::cout << "  Type: " << e.errorType << std::endl;
        std::cout << "  Message: " << e.errorMessage << std::endl;
        std::cout << "  Severity: " << e.severity << std::endl;
    }

    return 0;
}

