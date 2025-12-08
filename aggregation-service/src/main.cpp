#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <csignal>

#include "aggregator.h"
#include "database.h"
#include "handlers.h"

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
                                   " password=" + password;
    return connectionString;
}

int main() {
    std::cout << "Aggregation Service started" << std::endl;

    // Обработка сигналов для graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

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

    // Запускаем HTTP сервер в отдельном потоке
    std::thread httpThread([&httpServer, &httpHost, httpPort]() {
        std::cout << "HTTP server listening on " << httpHost << ":" << httpPort << std::endl;
        httpServer.listen(httpHost, httpPort);
    });

    // Запускаем агрегацию
    aggregation::Aggregator aggregator(database);
    aggregator.run();

    // Ожидаем завершения (для демонстрации выполняем один раз)
    // В реальном сценарии здесь был бы цикл агрегации

    std::cout << "Aggregation completed. HTTP server running. Press Ctrl+C to stop." << std::endl;

    // Ожидаем сигнала завершения
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Останавливаем HTTP сервер
    httpServer.stop();
    if (httpThread.joinable()) {
        httpThread.join();
    }

    std::cout << "Aggregation Service stopped" << std::endl;
    return 0;
}
