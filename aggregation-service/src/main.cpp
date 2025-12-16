#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <csignal>

#include "aggregator.h"
#include "database.h"
#include "handlers.h"
#include "metrics_client.h"
#include "aggregation_server.h"

static std::atomic<bool> running{true};

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

std::string GetEnvVar(const std::string& varName, const std::string& defaultValue) {
    const char* value = std::getenv(varName.c_str());
    if (value == nullptr) {
        return defaultValue;
    }
    return std::string(value);
}

std::string BuildPostrgresConnectionString() {
    std::string host = GetEnvVar("AGG_DB_HOST", "localhost");
    std::string port = GetEnvVar("AGG_DB_PORT", "5434");
    std::string dbname = GetEnvVar("AGG_DB_NAME", "aggregation_db");
    std::string user = GetEnvVar("AGG_DB_USER", "agguser");
    std::string password = GetEnvVar("AGG_DB_PASSWORD", "aggpassword");

    std::string connectionString = "host=" + host +
                                   " port=" + port +
                                   " dbname=" + dbname +
                                   " user=" + user +
                                   " password=" + password;//
    return connectionString;
}

int main() {
    std::cout << "Aggregation Service started" << std::endl;

    // Обработка сигналов для graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Интервал агрегации в секундах (по умолчанию 60 секунд)
    int aggregationIntervalSec = std::stoi(GetEnvVar("AGGREGATION_INTERVAL_SEC", "60"));
    std::cout << "Aggregation interval set to " << aggregationIntervalSec << " seconds" << std::endl;

    std::string connectionString = BuildPostrgresConnectionString();

    aggregation::Database database;
    if (!database.connect(connectionString)) {
        std::cerr << "Failed to connect to the database. Exiting." << std::endl;
        return 1;
    }

    if (!database.initializeSchema()) {
        std::cerr << "Failed to initialize database schema. Exiting." << std::endl;
        return 1;
    }

    // HTTP сервер для health checks
    httplib::Server httpServer;
    aggregation::HttpHandlers handlers(database);
    handlers.registerRoutes(httpServer);

    std::string httpHost = GetEnvVar("AGG_HTTP_HOST", "0.0.0.0");
    int httpPort = std::stoi(GetEnvVar("AGG_HTTP_PORT", "8081"));

    // gRPC подключение к metrics-service
    std::string metricsHost = GetEnvVar("METRICS_GRPC_HOST", "localhost");
    std::string metricsPort = GetEnvVar("METRICS_GRPC_PORT", "50051");

    std::cout << "Connecting to metrics-service via gRPC at "
              << metricsHost << ":" << metricsPort << std::endl;

    aggregation::MetricsClient metricsClient(metricsHost, metricsPort);

    if (metricsClient.isConnected()) {
        std::cout << "MetricsClient: gRPC channel is ready" << std::endl;
    } else {
        std::cout << "MetricsClient: gRPC channel is not ready yet (will connect on first request)" << std::endl;
    }

    // Создаем агрегатор
    aggregation::Aggregator aggregator(database, metricsClient);

    // Запускаем gRPC сервер для предоставления агрегированных данных
    std::string grpcHost = GetEnvVar("AGG_GRPC_HOST", "0.0.0.0");
    std::string grpcPort = GetEnvVar("AGG_GRPC_PORT", "50052");
    std::string grpcAddress = grpcHost + ":" + grpcPort;

    aggregation::AggregationGrpcServer grpcServer(database, grpcAddress);

    std::thread grpcThread([&grpcServer]() {
        grpcServer.start();
        grpcServer.wait();
    });

    // Запускаем HTTP сервер в отдельном потоке
    std::thread httpThread([&httpServer, &httpHost, httpPort]() {
        std::cout << "HTTP server listening on " << httpHost << ":" << httpPort << std::endl;
        httpServer.listen(httpHost, httpPort);
    });

    std::cout << "Starting periodic aggregation loop..." << std::endl;

    // Периодический цикл агрегации
    auto lastAggregationTime = std::chrono::steady_clock::now();

    while (running) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastAggregationTime);

        if (elapsed.count() >= aggregationIntervalSec) {
            std::cout << "\n=== Starting aggregation cycle ===" << std::endl;

            try {
                aggregator.run();
                std::cout << "=== Aggregation cycle completed successfully ===" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "ERROR: Aggregation cycle failed: " << e.what() << std::endl;
                std::cerr << "Will retry on next cycle..." << std::endl;
            } catch (...) {
                std::cerr << "ERROR: Aggregation cycle failed with unknown error" << std::endl;
                std::cerr << "Will retry on next cycle..." << std::endl;
            }

            lastAggregationTime = now;
        }

        // Короткий сон для проверки сигналов
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\nShutting down gracefully..." << std::endl;

    // Останавливаем gRPC сервер
    std::cout << "Stopping gRPC server..." << std::endl;
    grpcServer.stop();
    if (grpcThread.joinable()) {
        grpcThread.join();
    }

    // Останавливаем HTTP сервер
    std::cout << "Stopping HTTP server..." << std::endl;
    httpServer.stop();
    if (httpThread.joinable()) {
        httpThread.join();
    }

    std::cout << "Closing database connection..." << std::endl;
    database.disconnect();

    std::cout << "Aggregation Service stopped successfully" << std::endl;
    return 0;
}
