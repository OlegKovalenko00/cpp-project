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
