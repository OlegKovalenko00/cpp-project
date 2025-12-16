#include "models.hpp"

// ==================== ErrorResponse ====================

void to_json(nlohmann::json& j, const ErrorResponse& e) {
    j = nlohmann::json{{"code", e.code}, {"message", e.message}};
    if (e.details.has_value()) {
        j["details"] = e.details.value();
    }
}

void from_json(const nlohmann::json& j, ErrorResponse& e) {
    j.at("code").get_to(e.code);
    j.at("message").get_to(e.message);
    if (j.contains("details") && !j["details"].is_null()) {
        e.details = j["details"];
    }
}

// ==================== Monitoring ====================

void to_json(nlohmann::json& j, const UptimePeriodStat& s) {
    j = nlohmann::json{{"ok", s.ok}, {"total", s.total}, {"percent", s.percent}};
}

void from_json(const nlohmann::json& j, UptimePeriodStat& s) {
    j.at("ok").get_to(s.ok);
    j.at("total").get_to(s.total);
    j.at("percent").get_to(s.percent);
}

void to_json(nlohmann::json& j, const UptimePeriods& p) {
    j = nlohmann::json::object();
    if (p.day.has_value())
        j["day"] = p.day.value();
    if (p.week.has_value())
        j["week"] = p.week.value();
    if (p.month.has_value())
        j["month"] = p.month.value();
    if (p.year.has_value())
        j["year"] = p.year.value();
}

void from_json(const nlohmann::json& j, UptimePeriods& p) {
    if (j.contains("day") && !j["day"].is_null())
        p.day = j["day"].get<UptimePeriodStat>();
    if (j.contains("week") && !j["week"].is_null())
        p.week = j["week"].get<UptimePeriodStat>();
    if (j.contains("month") && !j["month"].is_null())
        p.month = j["month"].get<UptimePeriodStat>();
    if (j.contains("year") && !j["year"].is_null())
        p.year = j["year"].get<UptimePeriodStat>();
}

void to_json(nlohmann::json& j, const UptimeResponse& r) {
    j = nlohmann::json{{"service", r.service}, {"period", r.period}, {"periods", r.periods}};
}

void from_json(const nlohmann::json& j, UptimeResponse& r) {
    j.at("service").get_to(r.service);
    j.at("period").get_to(r.period);
    j.at("periods").get_to(r.periods);
}

// ==================== PageViewEvent ====================

void to_json(nlohmann::json& j, const PageViewEvent& e) {
    j = nlohmann::json{{"page", e.page}, {"timestamp", e.timestamp}};
    if (e.user_id.has_value())
        j["user_id"] = e.user_id.value();
    if (e.session_id.has_value())
        j["session_id"] = e.session_id.value();
    if (e.referrer.has_value())
        j["referrer"] = e.referrer.value();
}

void from_json(const nlohmann::json& j, PageViewEvent& e) {
    j.at("page").get_to(e.page);
    j.at("timestamp").get_to(e.timestamp);
    if (j.contains("user_id") && !j["user_id"].is_null())
        e.user_id = j["user_id"].get<std::string>();
    if (j.contains("session_id") && !j["session_id"].is_null())
        e.session_id = j["session_id"].get<std::string>();
    if (j.contains("referrer") && !j["referrer"].is_null())
        e.referrer = j["referrer"].get<std::string>();
}

// ==================== ClickEvent ====================

void to_json(nlohmann::json& j, const ClickEvent& e) {
    j = nlohmann::json{{"page", e.page}, {"element_id", e.element_id}, {"timestamp", e.timestamp}};
    if (e.action.has_value())
        j["action"] = e.action.value();
    if (e.user_id.has_value())
        j["user_id"] = e.user_id.value();
    if (e.session_id.has_value())
        j["session_id"] = e.session_id.value();
}

void from_json(const nlohmann::json& j, ClickEvent& e) {
    j.at("page").get_to(e.page);
    j.at("element_id").get_to(e.element_id);
    j.at("timestamp").get_to(e.timestamp);
    if (j.contains("action") && !j["action"].is_null())
        e.action = j["action"].get<std::string>();
    if (j.contains("user_id") && !j["user_id"].is_null())
        e.user_id = j["user_id"].get<std::string>();
    if (j.contains("session_id") && !j["session_id"].is_null())
        e.session_id = j["session_id"].get<std::string>();
}

// ==================== PerformanceEvent ====================

void to_json(nlohmann::json& j, const PerformanceEvent& e) {
    j = nlohmann::json{{"page", e.page}, {"timestamp", e.timestamp}};
    if (e.ttfb_ms.has_value())
        j["ttfb_ms"] = e.ttfb_ms.value();
    if (e.fcp_ms.has_value())
        j["fcp_ms"] = e.fcp_ms.value();
    if (e.lcp_ms.has_value())
        j["lcp_ms"] = e.lcp_ms.value();
    if (e.total_page_load_ms.has_value())
        j["total_page_load_ms"] = e.total_page_load_ms.value();
    if (e.user_id.has_value())
        j["user_id"] = e.user_id.value();
    if (e.session_id.has_value())
        j["session_id"] = e.session_id.value();
}

void from_json(const nlohmann::json& j, PerformanceEvent& e) {
    j.at("page").get_to(e.page);
    j.at("timestamp").get_to(e.timestamp);
    if (j.contains("ttfb_ms") && !j["ttfb_ms"].is_null())
        e.ttfb_ms = j["ttfb_ms"].get<double>();
    if (j.contains("fcp_ms") && !j["fcp_ms"].is_null())
        e.fcp_ms = j["fcp_ms"].get<double>();
    if (j.contains("lcp_ms") && !j["lcp_ms"].is_null())
        e.lcp_ms = j["lcp_ms"].get<double>();
    if (j.contains("total_page_load_ms") && !j["total_page_load_ms"].is_null())
        e.total_page_load_ms = j["total_page_load_ms"].get<double>();
    if (j.contains("user_id") && !j["user_id"].is_null())
        e.user_id = j["user_id"].get<std::string>();
    if (j.contains("session_id") && !j["session_id"].is_null())
        e.session_id = j["session_id"].get<std::string>();
}

// ==================== ErrorEvent ====================

void to_json(nlohmann::json& j, const ErrorEvent& e) {
    j = nlohmann::json{{"page", e.page},
                       {"error_type", e.error_type},
                       {"message", e.message},
                       {"severity", e.severity},
                       {"timestamp", e.timestamp}};
    if (e.stack.has_value())
        j["stack"] = e.stack.value();
    if (e.user_id.has_value())
        j["user_id"] = e.user_id.value();
    if (e.session_id.has_value())
        j["session_id"] = e.session_id.value();
}

void from_json(const nlohmann::json& j, ErrorEvent& e) {
    j.at("page").get_to(e.page);
    j.at("error_type").get_to(e.error_type);
    j.at("message").get_to(e.message);
    j.at("timestamp").get_to(e.timestamp);
    if (j.contains("stack") && !j["stack"].is_null())
        e.stack = j["stack"].get<std::string>();
    if (j.contains("severity") && !j["severity"].is_null())
        e.severity = j["severity"].get<Severity>();
    if (j.contains("user_id") && !j["user_id"].is_null())
        e.user_id = j["user_id"].get<std::string>();
    if (j.contains("session_id") && !j["session_id"].is_null())
        e.session_id = j["session_id"].get<std::string>();
}

// ==================== CustomEvent ====================

void to_json(nlohmann::json& j, const CustomEvent& e) {
    j = nlohmann::json{{"name", e.name}, {"timestamp", e.timestamp}};
    if (e.page.has_value())
        j["page"] = e.page.value();
    if (e.user_id.has_value())
        j["user_id"] = e.user_id.value();
    if (e.session_id.has_value())
        j["session_id"] = e.session_id.value();
    if (e.properties.has_value())
        j["properties"] = e.properties.value();
}

void from_json(const nlohmann::json& j, CustomEvent& e) {
    j.at("name").get_to(e.name);
    j.at("timestamp").get_to(e.timestamp);
    if (j.contains("page") && !j["page"].is_null())
        e.page = j["page"].get<std::string>();
    if (j.contains("user_id") && !j["user_id"].is_null())
        e.user_id = j["user_id"].get<std::string>();
    if (j.contains("session_id") && !j["session_id"].is_null())
        e.session_id = j["session_id"].get<std::string>();
    if (j.contains("properties") && !j["properties"].is_null())
        e.properties = j["properties"];
}

// ==================== Aggregation Common ====================

void to_json(nlohmann::json& j, const TimeRange& r) {
    j = nlohmann::json{{"from", r.from}, {"to", r.to}};
}

void from_json(const nlohmann::json& j, TimeRange& r) {
    j.at("from").get_to(r.from);
    j.at("to").get_to(r.to);
}

void to_json(nlohmann::json& j, const Pagination& p) {
    j = nlohmann::json::object();
    if (p.limit.has_value())
        j["limit"] = p.limit.value();
    if (p.offset.has_value())
        j["offset"] = p.offset.value();
}

void from_json(const nlohmann::json& j, Pagination& p) {
    if (j.contains("limit") && !j["limit"].is_null())
        p.limit = j["limit"].get<uint32_t>();
    if (j.contains("offset") && !j["offset"].is_null())
        p.offset = j["offset"].get<uint32_t>();
}

void to_json(nlohmann::json& j, const GetWatermarkResponse& r) {
    j = nlohmann::json::object();
    if (r.last_aggregated_at.has_value()) {
        j["last_aggregated_at"] = r.last_aggregated_at.value();
    } else {
        j["last_aggregated_at"] = nullptr;
    }
}

void from_json(const nlohmann::json& j, GetWatermarkResponse& r) {
    if (j.contains("last_aggregated_at") && !j["last_aggregated_at"].is_null()) {
        r.last_aggregated_at = j["last_aggregated_at"].get<std::string>();
    }
}

// ==================== Aggregation Rows ====================

void to_json(nlohmann::json& j, const AggPageViewsRow& r) {
    j = nlohmann::json{{"time_bucket", r.time_bucket},
                       {"project_id", r.project_id},
                       {"page", r.page},
                       {"views_count", r.views_count},
                       {"unique_users", r.unique_users},
                       {"unique_sessions", r.unique_sessions},
                       {"created_at", r.created_at}};
}

void from_json(const nlohmann::json& j, AggPageViewsRow& r) {
    j.at("time_bucket").get_to(r.time_bucket);
    j.at("project_id").get_to(r.project_id);
    j.at("page").get_to(r.page);
    j.at("views_count").get_to(r.views_count);
    j.at("unique_users").get_to(r.unique_users);
    j.at("unique_sessions").get_to(r.unique_sessions);
    j.at("created_at").get_to(r.created_at);
}

void to_json(nlohmann::json& j, const AggClicksRow& r) {
    j = nlohmann::json{{"time_bucket", r.time_bucket},
                       {"project_id", r.project_id},
                       {"page", r.page},
                       {"clicks_count", r.clicks_count},
                       {"unique_users", r.unique_users},
                       {"unique_sessions", r.unique_sessions},
                       {"created_at", r.created_at}};
    if (r.element_id.has_value())
        j["element_id"] = r.element_id.value();
}

void from_json(const nlohmann::json& j, AggClicksRow& r) {
    j.at("time_bucket").get_to(r.time_bucket);
    j.at("project_id").get_to(r.project_id);
    j.at("page").get_to(r.page);
    j.at("clicks_count").get_to(r.clicks_count);
    j.at("unique_users").get_to(r.unique_users);
    j.at("unique_sessions").get_to(r.unique_sessions);
    j.at("created_at").get_to(r.created_at);
    if (j.contains("element_id") && !j["element_id"].is_null())
        r.element_id = j["element_id"].get<std::string>();
}

void to_json(nlohmann::json& j, const AggPerformanceRow& r) {
    j = nlohmann::json{{"time_bucket", r.time_bucket},
                       {"project_id", r.project_id},
                       {"page", r.page},
                       {"samples_count", r.samples_count},
                       {"avg_total_load_ms", r.avg_total_load_ms},
                       {"p95_total_load_ms", r.p95_total_load_ms},
                       {"avg_ttfb_ms", r.avg_ttfb_ms},
                       {"p95_ttfb_ms", r.p95_ttfb_ms},
                       {"avg_fcp_ms", r.avg_fcp_ms},
                       {"p95_fcp_ms", r.p95_fcp_ms},
                       {"avg_lcp_ms", r.avg_lcp_ms},
                       {"p95_lcp_ms", r.p95_lcp_ms},
                       {"created_at", r.created_at}};
}

void from_json(const nlohmann::json& j, AggPerformanceRow& r) {
    j.at("time_bucket").get_to(r.time_bucket);
    j.at("project_id").get_to(r.project_id);
    j.at("page").get_to(r.page);
    j.at("samples_count").get_to(r.samples_count);
    j.at("avg_total_load_ms").get_to(r.avg_total_load_ms);
    j.at("p95_total_load_ms").get_to(r.p95_total_load_ms);
    j.at("avg_ttfb_ms").get_to(r.avg_ttfb_ms);
    j.at("p95_ttfb_ms").get_to(r.p95_ttfb_ms);
    j.at("avg_fcp_ms").get_to(r.avg_fcp_ms);
    j.at("p95_fcp_ms").get_to(r.p95_fcp_ms);
    j.at("avg_lcp_ms").get_to(r.avg_lcp_ms);
    j.at("p95_lcp_ms").get_to(r.p95_lcp_ms);
    j.at("created_at").get_to(r.created_at);
}

void to_json(nlohmann::json& j, const AggErrorsRow& r) {
    j = nlohmann::json{{"time_bucket", r.time_bucket},
                       {"project_id", r.project_id},
                       {"page", r.page},
                       {"errors_count", r.errors_count},
                       {"warning_count", r.warning_count},
                       {"critical_count", r.critical_count},
                       {"unique_users", r.unique_users},
                       {"created_at", r.created_at}};
    if (r.error_type.has_value())
        j["error_type"] = r.error_type.value();
}

void from_json(const nlohmann::json& j, AggErrorsRow& r) {
    j.at("time_bucket").get_to(r.time_bucket);
    j.at("project_id").get_to(r.project_id);
    j.at("page").get_to(r.page);
    j.at("errors_count").get_to(r.errors_count);
    j.at("warning_count").get_to(r.warning_count);
    j.at("critical_count").get_to(r.critical_count);
    j.at("unique_users").get_to(r.unique_users);
    j.at("created_at").get_to(r.created_at);
    if (j.contains("error_type") && !j["error_type"].is_null())
        r.error_type = j["error_type"].get<std::string>();
}

void to_json(nlohmann::json& j, const AggCustomEventsRow& r) {
    j = nlohmann::json{{"time_bucket", r.time_bucket},
                       {"project_id", r.project_id},
                       {"event_name", r.event_name},
                       {"events_count", r.events_count},
                       {"unique_users", r.unique_users},
                       {"unique_sessions", r.unique_sessions},
                       {"created_at", r.created_at}};
    if (r.page.has_value())
        j["page"] = r.page.value();
}

void from_json(const nlohmann::json& j, AggCustomEventsRow& r) {
    j.at("time_bucket").get_to(r.time_bucket);
    j.at("project_id").get_to(r.project_id);
    j.at("event_name").get_to(r.event_name);
    j.at("events_count").get_to(r.events_count);
    j.at("unique_users").get_to(r.unique_users);
    j.at("unique_sessions").get_to(r.unique_sessions);
    j.at("created_at").get_to(r.created_at);
    if (j.contains("page") && !j["page"].is_null())
        r.page = j["page"].get<std::string>();
}

// ==================== Aggregation Requests ====================

void to_json(nlohmann::json& j, const GetPageViewsAggRequest& r) {
    j = nlohmann::json{{"project_id", r.project_id},
                       {"time_range", r.time_range}};
    if (r.page.has_value())
        j["page"] = r.page.value();
    if (r.pagination.has_value())
        j["pagination"] = r.pagination.value();
}

void from_json(const nlohmann::json& j, GetPageViewsAggRequest& r) {
    j.at("project_id").get_to(r.project_id);
    j.at("time_range").get_to(r.time_range);
    if (j.contains("page") && !j["page"].is_null())
        r.page = j["page"].get<std::string>();
    if (j.contains("pagination") && !j["pagination"].is_null())
        r.pagination = j["pagination"].get<Pagination>();
}

void to_json(nlohmann::json& j, const GetClicksAggRequest& r) {
    j = nlohmann::json{{"project_id", r.project_id},
                       {"time_range", r.time_range}};
    if (r.page.has_value())
        j["page"] = r.page.value();
    if (r.element_id.has_value())
        j["element_id"] = r.element_id.value();
    if (r.pagination.has_value())
        j["pagination"] = r.pagination.value();
}

void from_json(const nlohmann::json& j, GetClicksAggRequest& r) {
    j.at("project_id").get_to(r.project_id);
    j.at("time_range").get_to(r.time_range);
    if (j.contains("page") && !j["page"].is_null())
        r.page = j["page"].get<std::string>();
    if (j.contains("element_id") && !j["element_id"].is_null())
        r.element_id = j["element_id"].get<std::string>();
    if (j.contains("pagination") && !j["pagination"].is_null())
        r.pagination = j["pagination"].get<Pagination>();
}

void to_json(nlohmann::json& j, const GetPerformanceAggRequest& r) {
    j = nlohmann::json{{"project_id", r.project_id},
                       {"time_range", r.time_range}};
    if (r.page.has_value())
        j["page"] = r.page.value();
    if (r.pagination.has_value())
        j["pagination"] = r.pagination.value();
}

void from_json(const nlohmann::json& j, GetPerformanceAggRequest& r) {
    j.at("project_id").get_to(r.project_id);
    j.at("time_range").get_to(r.time_range);
    if (j.contains("page") && !j["page"].is_null())
        r.page = j["page"].get<std::string>();
    if (j.contains("pagination") && !j["pagination"].is_null())
        r.pagination = j["pagination"].get<Pagination>();
}

void to_json(nlohmann::json& j, const GetErrorsAggRequest& r) {
    j = nlohmann::json{{"project_id", r.project_id},
                       {"time_range", r.time_range}};
    if (r.page.has_value())
        j["page"] = r.page.value();
    if (r.error_type.has_value())
        j["error_type"] = r.error_type.value();
    if (r.pagination.has_value())
        j["pagination"] = r.pagination.value();
}

void from_json(const nlohmann::json& j, GetErrorsAggRequest& r) {
    j.at("project_id").get_to(r.project_id);
    j.at("time_range").get_to(r.time_range);
    if (j.contains("page") && !j["page"].is_null())
        r.page = j["page"].get<std::string>();
    if (j.contains("error_type") && !j["error_type"].is_null())
        r.error_type = j["error_type"].get<std::string>();
    if (j.contains("pagination") && !j["pagination"].is_null())
        r.pagination = j["pagination"].get<Pagination>();
}

void to_json(nlohmann::json& j, const GetCustomEventsAggRequest& r) {
    j = nlohmann::json{{"project_id", r.project_id},
                       {"time_range", r.time_range},
                       {"event_name", r.event_name}};
    if (r.page.has_value())
        j["page"] = r.page.value();
    if (r.pagination.has_value())
        j["pagination"] = r.pagination.value();
}

void from_json(const nlohmann::json& j, GetCustomEventsAggRequest& r) {
    j.at("project_id").get_to(r.project_id);
    j.at("time_range").get_to(r.time_range);
    j.at("event_name").get_to(r.event_name);
    if (j.contains("page") && !j["page"].is_null())
        r.page = j["page"].get<std::string>();
    if (j.contains("pagination") && !j["pagination"].is_null())
        r.pagination = j["pagination"].get<Pagination>();
}

// ==================== Aggregation Responses ====================

void to_json(nlohmann::json& j, const GetPageViewsAggResponse& r) {
    j = nlohmann::json{{"rows", r.rows}};
}

void from_json(const nlohmann::json& j, GetPageViewsAggResponse& r) {
    j.at("rows").get_to(r.rows);
}

void to_json(nlohmann::json& j, const GetClicksAggResponse& r) {
    j = nlohmann::json{{"rows", r.rows}};
}

void from_json(const nlohmann::json& j, GetClicksAggResponse& r) {
    j.at("rows").get_to(r.rows);
}

void to_json(nlohmann::json& j, const GetPerformanceAggResponse& r) {
    j = nlohmann::json{{"rows", r.rows}};
}

void from_json(const nlohmann::json& j, GetPerformanceAggResponse& r) {
    j.at("rows").get_to(r.rows);
}

void to_json(nlohmann::json& j, const GetErrorsAggResponse& r) {
    j = nlohmann::json{{"rows", r.rows}};
}

void from_json(const nlohmann::json& j, GetErrorsAggResponse& r) {
    j.at("rows").get_to(r.rows);
}

void to_json(nlohmann::json& j, const GetCustomEventsAggResponse& r) {
    j = nlohmann::json{{"rows", r.rows}};
}

void from_json(const nlohmann::json& j, GetCustomEventsAggResponse& r) {
    j.at("rows").get_to(r.rows);
}
