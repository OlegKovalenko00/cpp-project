#pragma once

#include <string>
#include <optional>
#include <cstdint>

struct DatabaseConfig {
    std::string host;
    std::string dbname;
    std::string user;
    std::string password;
};

// Data structures for metrics
struct PageView {
    std::string page;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    std::optional<std::string> referrer;
};

struct ClickEvent {
    std::string page;
    std::optional<std::string> element_id;
    std::optional<std::string> action;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
};

struct PerformanceEvent {
    std::string page;
    std::optional<double> ttfb_ms;
    std::optional<double> fcp_ms;
    std::optional<double> lcp_ms;
    std::optional<double> total_page_load_ms;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
};

struct ErrorEvent {
    std::string page;
    std::optional<std::string> error_type;
    std::optional<std::string> message;
    std::optional<std::string> stack;
    std::optional<int32_t> severity;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
};

struct CustomEvent {
    std::string name;
    std::optional<std::string> page;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
};

// Configuration functions
DatabaseConfig load_database_config();
bool test_database_connection(const DatabaseConfig& config);

// INSERT functions
bool save_page_view(const DatabaseConfig& config, const PageView& event);
bool save_click_event(const DatabaseConfig& config, const ClickEvent& event);
bool save_performance_event(const DatabaseConfig& config, const PerformanceEvent& event);
bool save_error_event(const DatabaseConfig& config, const ErrorEvent& event);
bool save_custom_event(const DatabaseConfig& config, const CustomEvent& event);
