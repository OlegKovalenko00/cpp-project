#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

// ==================== Enums ====================

enum class ErrorCode {
    InvalidPageView,
    InvalidClickEvent,
    InvalidPerformanceEvent,
    InvalidErrorEvent,
    InvalidCustomEvent,
    InvalidBatch,
    ValidationError,
    InternalError
};

NLOHMANN_JSON_SERIALIZE_ENUM(ErrorCode,
                             {{ErrorCode::InvalidPageView, "INVALID_PAGE_VIEW"},
                              {ErrorCode::InvalidClickEvent, "INVALID_CLICK_EVENT"},
                              {ErrorCode::InvalidPerformanceEvent, "INVALID_PERFORMANCE_EVENT"},
                              {ErrorCode::InvalidErrorEvent, "INVALID_ERROR_EVENT"},
                              {ErrorCode::InvalidCustomEvent, "INVALID_CUSTOM_EVENT"},
                              {ErrorCode::InvalidBatch, "INVALID_BATCH"},
                              {ErrorCode::ValidationError, "VALIDATION_ERROR"},
                              {ErrorCode::InternalError, "INTERNAL_ERROR"}})

enum class Severity { Warning, Error, Critical };

NLOHMANN_JSON_SERIALIZE_ENUM(Severity, {{Severity::Warning, "warning"},
                                        {Severity::Error, "error"},
                                        {Severity::Critical, "critical"}})

// ==================== Schemas ====================

// HealthResponse
struct HealthResponse {
    std::string status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(HealthResponse, status)
};

// ErrorResponse
struct ErrorResponse {
    ErrorCode code;
    std::string message;
    std::optional<nlohmann::json> details;
};

void to_json(nlohmann::json& j, const ErrorResponse& e);
void from_json(const nlohmann::json& j, ErrorResponse& e);

// PageViewEvent
struct PageViewEvent {
    std::string page;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    std::optional<std::string> referrer;
    int64_t timestamp;
};

void to_json(nlohmann::json& j, const PageViewEvent& e);
void from_json(const nlohmann::json& j, PageViewEvent& e);

// ClickEvent
struct ClickEvent {
    std::string page;
    std::string element_id;
    std::optional<std::string> action;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    int64_t timestamp;
};

void to_json(nlohmann::json& j, const ClickEvent& e);
void from_json(const nlohmann::json& j, ClickEvent& e);

// PerformanceEvent
struct PerformanceEvent {
    std::string page;
    std::optional<double> ttfb_ms;
    std::optional<double> fcp_ms;
    std::optional<double> lcp_ms;
    std::optional<double> total_page_load_ms;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    int64_t timestamp;
};

void to_json(nlohmann::json& j, const PerformanceEvent& e);
void from_json(const nlohmann::json& j, PerformanceEvent& e);

// ErrorEvent
struct ErrorEvent {
    std::string page;
    std::string error_type;
    std::string message;
    std::optional<std::string> stack;
    Severity severity = Severity::Error;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    int64_t timestamp;
};

void to_json(nlohmann::json& j, const ErrorEvent& e);
void from_json(const nlohmann::json& j, ErrorEvent& e);

// CustomEvent
struct CustomEvent {
    std::string name;
    std::optional<std::string> page;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    std::optional<nlohmann::json> properties;
    int64_t timestamp;
};

void to_json(nlohmann::json& j, const CustomEvent& e);
void from_json(const nlohmann::json& j, CustomEvent& e);
