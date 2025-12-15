#include <iostream>
#include <cstdlib>
#include <thread>
#include <nlohmann/json.hpp>
#include "database.h"
#include "metrics.h"
#include "rabbitmq.h"
#include "http_handler.h"

using json = nlohmann::json;

void process_page_view(const json& data, const DatabaseConfig& db_config) {
    PageView event;
    event.page = data.value("page", "");
    if (data.contains("user_id") && !data["user_id"].is_null()) 
        event.user_id = data["user_id"].get<std::string>();
    if (data.contains("session_id") && !data["session_id"].is_null()) 
        event.session_id = data["session_id"].get<std::string>();
    if (data.contains("referrer") && !data["referrer"].is_null()) 
        event.referrer = data["referrer"].get<std::string>();
    
    save_page_view(db_config, event);
}

void process_click_event(const json& data, const DatabaseConfig& db_config) {
    ClickEvent event;
    event.page = data.value("page", "");
    if (data.contains("element_id") && !data["element_id"].is_null()) 
        event.element_id = data["element_id"].get<std::string>();
    if (data.contains("action") && !data["action"].is_null()) 
        event.action = data["action"].get<std::string>();
    if (data.contains("user_id") && !data["user_id"].is_null()) 
        event.user_id = data["user_id"].get<std::string>();
    if (data.contains("session_id") && !data["session_id"].is_null()) 
        event.session_id = data["session_id"].get<std::string>();
    
    save_click_event(db_config, event);
}

void process_performance_event(const json& data, const DatabaseConfig& db_config) {
    PerformanceEvent event;
    event.page = data.value("page", "");
    if (data.contains("ttfb_ms") && !data["ttfb_ms"].is_null()) 
        event.ttfb_ms = data["ttfb_ms"].get<double>();
    if (data.contains("fcp_ms") && !data["fcp_ms"].is_null()) 
        event.fcp_ms = data["fcp_ms"].get<double>();
    if (data.contains("lcp_ms") && !data["lcp_ms"].is_null()) 
        event.lcp_ms = data["lcp_ms"].get<double>();
    if (data.contains("total_page_load_ms") && !data["total_page_load_ms"].is_null()) 
        event.total_page_load_ms = data["total_page_load_ms"].get<double>();
    if (data.contains("user_id") && !data["user_id"].is_null()) 
        event.user_id = data["user_id"].get<std::string>();
    if (data.contains("session_id") && !data["session_id"].is_null()) 
        event.session_id = data["session_id"].get<std::string>();
    
    save_performance_event(db_config, event);
}

void process_error_event(const json& data, const DatabaseConfig& db_config) {
    ErrorEvent event;
    event.page = data.value("page", "");
    if (data.contains("error_type") && !data["error_type"].is_null()) 
        event.error_type = data["error_type"].get<std::string>();
    if (data.contains("message") && !data["message"].is_null()) 
        event.message = data["message"].get<std::string>();
    if (data.contains("stack") && !data["stack"].is_null()) 
        event.stack = data["stack"].get<std::string>();
    if (data.contains("severity") && !data["severity"].is_null()) {
        // Handle both string and int severity
        if (data["severity"].is_string()) {
            std::string sev = data["severity"].get<std::string>();
            if (sev == "warning") event.severity = 0;
            else if (sev == "error") event.severity = 1;
            else if (sev == "critical") event.severity = 2;
        } else {
            event.severity = data["severity"].get<int32_t>();
        }
    }
    if (data.contains("user_id") && !data["user_id"].is_null()) 
        event.user_id = data["user_id"].get<std::string>();
    if (data.contains("session_id") && !data["session_id"].is_null()) 
        event.session_id = data["session_id"].get<std::string>();
    
    save_error_event(db_config, event);
}

void process_custom_event(const json& data, const DatabaseConfig& db_config) {
    CustomEvent event;
    event.name = data.value("name", "");
    if (data.contains("page") && !data["page"].is_null()) 
        event.page = data["page"].get<std::string>();
    if (data.contains("user_id") && !data["user_id"].is_null()) 
        event.user_id = data["user_id"].get<std::string>();
    if (data.contains("session_id") && !data["session_id"].is_null()) 
        event.session_id = data["session_id"].get<std::string>();
    
    save_custom_event(db_config, event);
}

void process_message(const std::string& queue, const std::string& message, const DatabaseConfig& db_config) {
    std::cout << "Received message from queue '" << queue << "': " << message << std::endl;
    
    try {
        json data = json::parse(message);
        
        if (queue == "page_views") {
            process_page_view(data, db_config);
        } else if (queue == "clicks") {
            process_click_event(data, db_config);
        } else if (queue == "performance_events") {
            process_performance_event(data, db_config);
        } else if (queue == "error_events") {
            process_error_event(data, db_config);
        } else if (queue == "custom_events") {
            process_custom_event(data, db_config);
        } else {
            std::cerr << "Unknown queue: " << queue << std::endl;
        }
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Metrics service starting..." << std::endl;

    DatabaseConfig db_config = load_database_config();
    std::cout << "Testing database connection..." << std::endl;
    if (!test_database_connection(db_config)) {
        std::cerr << "Failed to connect to database. Exiting." << std::endl;
        return 1;
    }
    std::cout << "Database connection successful." << std::endl;

    const char* http_port_env = std::getenv("HTTP_PORT");
    int http_port = http_port_env ? std::stoi(http_port_env) : 8080;
    HttpHandler http_handler(http_port, [&db_config]() {
        return test_database_connection(db_config);
    });
    http_handler.start();

    RabbitMQConfig rabbit_config = load_rabbitmq_config();
    RabbitMQConsumer rabbit(rabbit_config);
    std::cout << "Connecting to RabbitMQ at " << rabbit_config.host << ":" << rabbit_config.port << std::endl;
    rabbit.connect();
    rabbit.subscribe();
    rabbit.start();

    // Worker pool for async message processing
    const int WORKER_COUNT = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 4;
    std::vector<std::thread> workers;
    auto& msg_queue = rabbit.get_message_queue();
    std::atomic<bool> running{true};
    for (int i = 0; i < WORKER_COUNT; ++i) {
        workers.emplace_back([&db_config, &msg_queue, &running]() {
            while (running) {
                RabbitMQMessage msg;
                if (msg_queue.pop(msg)) {
                    process_message(msg.queue, msg.body, db_config);
                }
            }
        });
    }

    const char* port_env = std::getenv("GRPC_PORT");
    std::string port = port_env ? std::string(port_env) : "50051";
    std::string server_address = "0.0.0.0:" + port;
    std::cout << "Starting gRPC server on " << server_address << std::endl;
    run_grpc_server(server_address, db_config);

    // Shutdown
    running = false;
    msg_queue.stop();
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }
    rabbit.stop();
    http_handler.stop();
    return 0;
}
