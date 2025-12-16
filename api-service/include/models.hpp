#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <optional>
#include <string>
#include <cstdint>

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

// ==================== Aggregation Common ====================

struct TimeRange {
    std::string from;
    std::string to;
};

void to_json(nlohmann::json& j, const TimeRange& r);
void from_json(const nlohmann::json& j, TimeRange& r);

struct Pagination {
    std::optional<uint32_t> limit;
    std::optional<uint32_t> offset;
};

void to_json(nlohmann::json& j, const Pagination& p);
void from_json(const nlohmann::json& j, Pagination& p);

struct GetWatermarkResponse {
    std::optional<std::string> last_aggregated_at;
};

void to_json(nlohmann::json& j, const GetWatermarkResponse& r);
void from_json(const nlohmann::json& j, GetWatermarkResponse& r);

// ==================== Aggregation Rows ====================

struct AggPageViewsRow {
    std::string time_bucket;
    std::string project_id;
    std::string page;
    int64_t views_count;
    int64_t unique_users;
    int64_t unique_sessions;
    std::string created_at;
};

void to_json(nlohmann::json& j, const AggPageViewsRow& r);
void from_json(const nlohmann::json& j, AggPageViewsRow& r);

struct AggClicksRow {
    std::string time_bucket;
    std::string project_id;
    std::string page;
    std::optional<std::string> element_id;
    int64_t clicks_count;
    int64_t unique_users;
    int64_t unique_sessions;
    std::string created_at;
};

void to_json(nlohmann::json& j, const AggClicksRow& r);
void from_json(const nlohmann::json& j, AggClicksRow& r);

struct AggPerformanceRow {
    std::string time_bucket;
    std::string project_id;
    std::string page;
    int64_t samples_count;
    double avg_total_load_ms;
    double p95_total_load_ms;
    double avg_ttfb_ms;
    double p95_ttfb_ms;
    double avg_fcp_ms;
    double p95_fcp_ms;
    double avg_lcp_ms;
    double p95_lcp_ms;
    std::string created_at;
};

void to_json(nlohmann::json& j, const AggPerformanceRow& r);
void from_json(const nlohmann::json& j, AggPerformanceRow& r);

struct AggErrorsRow {
    std::string time_bucket;
    std::string project_id;
    std::string page;
    std::optional<std::string> error_type;
    int64_t errors_count;
    int64_t warning_count;
    int64_t critical_count;
    int64_t unique_users;
    std::string created_at;
};

void to_json(nlohmann::json& j, const AggErrorsRow& r);
void from_json(const nlohmann::json& j, AggErrorsRow& r);

struct AggCustomEventsRow {
    std::string time_bucket;
    std::string project_id;
    std::string event_name;
    std::optional<std::string> page;
    int64_t events_count;
    int64_t unique_users;
    int64_t unique_sessions;
    std::string created_at;
};

void to_json(nlohmann::json& j, const AggCustomEventsRow& r);
void from_json(const nlohmann::json& j, AggCustomEventsRow& r);

// ==================== Aggregation Requests ====================

struct GetPageViewsAggRequest {
    std::string project_id;
    TimeRange time_range;
    std::optional<std::string> page;
    std::optional<Pagination> pagination;
};

void to_json(nlohmann::json& j, const GetPageViewsAggRequest& r);
void from_json(const nlohmann::json& j, GetPageViewsAggRequest& r);

struct GetClicksAggRequest {
    std::string project_id;
    TimeRange time_range;
    std::optional<std::string> page;
    std::optional<std::string> element_id;
    std::optional<Pagination> pagination;
};

void to_json(nlohmann::json& j, const GetClicksAggRequest& r);
void from_json(const nlohmann::json& j, GetClicksAggRequest& r);

struct GetPerformanceAggRequest {
    std::string project_id;
    TimeRange time_range;
    std::optional<std::string> page;
    std::optional<Pagination> pagination;
};

void to_json(nlohmann::json& j, const GetPerformanceAggRequest& r);
void from_json(const nlohmann::json& j, GetPerformanceAggRequest& r);

struct GetErrorsAggRequest {
    std::string project_id;
    TimeRange time_range;
    std::optional<std::string> page;
    std::optional<std::string> error_type;
    std::optional<Pagination> pagination;
};

void to_json(nlohmann::json& j, const GetErrorsAggRequest& r);
void from_json(const nlohmann::json& j, GetErrorsAggRequest& r);

struct GetCustomEventsAggRequest {
    std::string project_id;
    TimeRange time_range;
    std::string event_name;
    std::optional<std::string> page;
    std::optional<Pagination> pagination;
};

void to_json(nlohmann::json& j, const GetCustomEventsAggRequest& r);
void from_json(const nlohmann::json& j, GetCustomEventsAggRequest& r);

// ==================== Aggregation Responses ====================

struct GetPageViewsAggResponse {
    std::vector<AggPageViewsRow> rows;
};

void to_json(nlohmann::json& j, const GetPageViewsAggResponse& r);
void from_json(const nlohmann::json& j, GetPageViewsAggResponse& r);

struct GetClicksAggResponse {
    std::vector<AggClicksRow> rows;
};

void to_json(nlohmann::json& j, const GetClicksAggResponse& r);
void from_json(const nlohmann::json& j, GetClicksAggResponse& r);

struct GetPerformanceAggResponse {
    std::vector<AggPerformanceRow> rows;
};

void to_json(nlohmann::json& j, const GetPerformanceAggResponse& r);
void from_json(const nlohmann::json& j, GetPerformanceAggResponse& r);

struct GetErrorsAggResponse {
    std::vector<AggErrorsRow> rows;
};

void to_json(nlohmann::json& j, const GetErrorsAggResponse& r);
void from_json(const nlohmann::json& j, GetErrorsAggResponse& r);

struct GetCustomEventsAggResponse {
    std::vector<AggCustomEventsRow> rows;
};

void to_json(nlohmann::json& j, const GetCustomEventsAggResponse& r);
void from_json(const nlohmann::json& j, GetCustomEventsAggResponse& r);
