#include "handlers.hpp"

#include <iostream>
#include <cstdlib>
#include <chrono>
#include <grpcpp/create_channel.h>
#include <google/protobuf/util/time_util.h>
#include "rabbitmq.hpp"
#include "aggregation_client.hpp"
#include "monitoring_client.hpp"

using json = nlohmann::json;

static std::string getEnvStr(const char* name, const std::string& defaultValue) {
    const char* val = std::getenv(name);
    return val ? val : defaultValue;
}

static int getEnvInt(const char* name, int defaultValue) {
    const char* val = std::getenv(name);
    return val ? std::stoi(val) : defaultValue;
}

static RabbitMQ* g_rabbitmq = nullptr;
static AggregationClient* g_aggregation_client = nullptr;
static MonitoringClient* g_monitoring_client = nullptr;

static RabbitMQ& getRabbitMQ() {
    if (!g_rabbitmq) {
        g_rabbitmq = new RabbitMQ(
            getEnvStr("RABBITMQ_HOST", "localhost"),
            getEnvInt("RABBITMQ_PORT", 5672),
            getEnvStr("RABBITMQ_USERNAME", "guest"),
            getEnvStr("RABBITMQ_PASSWORD", "guest"),
            getEnvStr("RABBITMQ_VHOST", "/")
        );
        if (!g_rabbitmq->connect()) {
            std::cerr << "[RabbitMQ] Failed to connect, messages will be lost" << std::endl;
        }
    }
    return *g_rabbitmq;
}

static AggregationClient& getAggregationClient() {
    if (!g_aggregation_client) {
        const std::string host = getEnvStr("AGGREGATION_GRPC_HOST", "localhost");
        const int port = getEnvInt("AGGREGATION_GRPC_PORT", 50052);
        auto channel = grpc::CreateChannel(host + ":" + std::to_string(port),
                                           grpc::InsecureChannelCredentials());
        g_aggregation_client = new AggregationClient(std::move(channel));
    }
    return *g_aggregation_client;
}

static MonitoringClient& getMonitoringClient() {
    if (!g_monitoring_client) {
        const std::string host = getEnvStr("MONITORING_HTTP_HOST", "localhost");
        const int port = getEnvInt("MONITORING_HTTP_PORT", 8083);
        const std::string baseUrl = "http://" + host + ":" + std::to_string(port);
        g_monitoring_client = new MonitoringClient(baseUrl);
    }
    return *g_monitoring_client;
}

// ==================== Helpers ====================

static bool parseIsoTimestamp(const std::string& iso, google::protobuf::Timestamp* ts) {
    return google::protobuf::util::TimeUtil::FromString(iso, ts);
}

static std::string toIsoString(const google::protobuf::Timestamp& ts) {
    return google::protobuf::util::TimeUtil::ToString(ts);
}

// ==================== Helper Functions ====================

static void sendError(httplib::Response& res, int status, ErrorCode code,
                      const std::string& message,
                      std::optional<json> details = std::nullopt) {
    ErrorResponse err{code, message, details};
    res.status = status;
    res.set_content(json(err).dump(), "application/json");
}

static void sendAccepted(httplib::Response& res) {
    res.status = 202;
    res.set_content(R"({"status":"accepted"})", "application/json");
}

// ==================== Handlers ====================

// GET /health/ping
void handleHealthPing(const httplib::Request& req, httplib::Response& res) {
    HealthResponse response{"ok"};
    res.status = 200;
    res.set_content(json(response).dump(), "application/json");
}

// POST /page-views
void handlePageView(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = json::parse(req.body);
        PageViewEvent event = body.get<PageViewEvent>();

        if (event.page.empty()) {
            sendError(res, 400, ErrorCode::InvalidPageView,
                      "Field 'page' must not be empty",
                      json{{"field", "page"}, {"reason", "required"}});
            return;
        }

        std::cout << "[PageView] page=" << event.page << std::endl;

        if (!getRabbitMQ().publish("", "page_views", json(event).dump())) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to publish page view event");
            return;
        } else {
            std::cout << "[PageView] Published event for page=" << event.page << std::endl;
        }
        sendAccepted(res);
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::InvalidPageView,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// POST /clicks
void handleClick(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = json::parse(req.body);
        ClickEvent event = body.get<ClickEvent>();

        if (event.page.empty()) {
            sendError(res, 400, ErrorCode::InvalidClickEvent,
                      "Field 'page' must not be empty",
                      json{{"field", "page"}, {"reason", "required"}});
            return;
        }
        if (event.element_id.empty()) {
            sendError(res, 400, ErrorCode::InvalidClickEvent,
                      "Field 'element_id' must not be empty",
                      json{{"field", "element_id"}, {"reason", "required"}});
            return;
        }

        std::cout << "[Click] page=" << event.page
                  << " element_id=" << event.element_id << std::endl;

        if (!getRabbitMQ().publish("", "clicks", json(event).dump())) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to publish click event");
            return;
        } else {
            std::cout << "[Click] Published event for page=" << event.page << " element_id=" << event.element_id << std::endl;
        }
        sendAccepted(res);
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::InvalidClickEvent,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// POST /performance
void handlePerformance(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = json::parse(req.body);
        PerformanceEvent event = body.get<PerformanceEvent>();

        if (event.page.empty()) {
            sendError(res, 400, ErrorCode::InvalidPerformanceEvent,
                      "Field 'page' must not be empty",
                      json{{"field", "page"}, {"reason", "required"}});
            return;
        }

        if (event.ttfb_ms.has_value() && event.ttfb_ms.value() < 0) {
            sendError(res, 400, ErrorCode::InvalidPerformanceEvent,
                      "Timing fields must be non-negative",
                      json{{"field", "ttfb_ms"}, {"reason", "must_be_positive"}});
            return;
        }
        if (event.fcp_ms.has_value() && event.fcp_ms.value() < 0) {
            sendError(res, 400, ErrorCode::InvalidPerformanceEvent,
                      "Timing fields must be non-negative",
                      json{{"field", "fcp_ms"}, {"reason", "must_be_positive"}});
            return;
        }
        if (event.lcp_ms.has_value() && event.lcp_ms.value() < 0) {
            sendError(res, 400, ErrorCode::InvalidPerformanceEvent,
                      "Timing fields must be non-negative",
                      json{{"field", "lcp_ms"}, {"reason", "must_be_positive"}});
            return;
        }
        if (event.total_page_load_ms.has_value() &&
            event.total_page_load_ms.value() < 0) {
            sendError(
                res, 400, ErrorCode::InvalidPerformanceEvent,
                "Timing fields must be non-negative",
                json{{"field", "total_page_load_ms"}, {"reason", "must_be_positive"}});
            return;
        }

        std::cout << "[Performance] page=" << event.page << std::endl;

        if (!getRabbitMQ().publish("", "performance_events", json(event).dump())) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to publish performance event");
            return;
        } else {
            std::cout << "[Performance] Published event for page=" << event.page << std::endl;
        }
        sendAccepted(res);
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::InvalidPerformanceEvent,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// POST /errors
void handleErrorEvent(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = json::parse(req.body);
        ErrorEvent event = body.get<ErrorEvent>();

        if (event.page.empty()) {
            sendError(res, 400, ErrorCode::InvalidErrorEvent,
                      "Field 'page' must not be empty",
                      json{{"field", "page"}, {"reason", "required"}});
            return;
        }
        if (event.error_type.empty()) {
            sendError(res, 400, ErrorCode::InvalidErrorEvent,
                      "Field 'error_type' must not be empty",
                      json{{"field", "error_type"}, {"reason", "required"}});
            return;
        }
        if (event.message.empty()) {
            sendError(res, 400, ErrorCode::InvalidErrorEvent,
                      "Field 'message' must not be empty",
                      json{{"field", "message"}, {"reason", "required"}});
            return;
        }

        std::cout << "[ErrorEvent] page=" << event.page
                  << " error_type=" << event.error_type << std::endl;

        if (!getRabbitMQ().publish("", "error_events", json(event).dump())) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to publish error event");
            return;
        } else {
            std::cout << " DSJKLFJDSLKJFL:DSKJFLKSDJFLKDJSFLKDSJFKLDF [ErrorEvent] Published event for page=" << event.page << " error_type=" << event.error_type << std::endl;
        }
        sendAccepted(res);
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::InvalidErrorEvent,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// POST /custom-events
void handleCustomEvent(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = json::parse(req.body);
        CustomEvent event = body.get<CustomEvent>();

        if (event.name.empty()) {
            sendError(res, 400, ErrorCode::InvalidCustomEvent,
                      "Field 'name' must not be empty",
                      json{{"field", "name"}, {"reason", "required"}});
            return;
        }

        std::cout << "[CustomEvent] name=" << event.name << std::endl;

        if (!getRabbitMQ().publish("", "custom_events", json(event).dump())) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to publish custom event");
            return;
        } else {
            std::cout << "[CustomEvent] Published event name=" << event.name << std::endl;
        }
        sendAccepted(res);
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::InvalidCustomEvent,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// GET /uptime
void handleUptime(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("MONITORING_HTTP_TIMEOUT_MS", 2000);
    if (!req.has_param("service")) {
        sendError(res, 400, ErrorCode::ValidationError,
                  "Missing query param: service");
        return;
    }
    const std::string service = req.get_param_value("service");
    std::optional<std::string> period;
    if (req.has_param("period")) {
        period = req.get_param_value("period");
    }

    UptimeResponse respBody;
    if (!getMonitoringClient().GetUptime(service, period, &respBody,
                                         std::chrono::milliseconds(timeoutMs))) {
        sendError(res, 502, ErrorCode::InternalError,
                  "Monitoring service unavailable");
        return;
    }
    res.status = 200;
    res.set_content(json(respBody).dump(), "application/json");
}

// GET /uptime/day|week|month|year
void handleUptimeDay(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("MONITORING_HTTP_TIMEOUT_MS", 2000);
    if (!req.has_param("service")) {
        sendError(res, 400, ErrorCode::ValidationError,
                  "Missing query param: service");
        return;
    }
    UptimeResponse respBody;
    if (!getMonitoringClient().GetUptimeDay(req.get_param_value("service"), &respBody,
                                            std::chrono::milliseconds(timeoutMs))) {
        sendError(res, 502, ErrorCode::InternalError,
                  "Monitoring service unavailable");
        return;
    }
    res.status = 200;
    res.set_content(json(respBody).dump(), "application/json");
}

void handleUptimeWeek(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("MONITORING_HTTP_TIMEOUT_MS", 2000);
    if (!req.has_param("service")) {
        sendError(res, 400, ErrorCode::ValidationError,
                  "Missing query param: service");
        return;
    }
    UptimeResponse respBody;
    if (!getMonitoringClient().GetUptimeWeek(req.get_param_value("service"), &respBody,
                                             std::chrono::milliseconds(timeoutMs))) {
        sendError(res, 502, ErrorCode::InternalError,
                  "Monitoring service unavailable");
        return;
    }
    res.status = 200;
    res.set_content(json(respBody).dump(), "application/json");
}

void handleUptimeMonth(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("MONITORING_HTTP_TIMEOUT_MS", 2000);
    if (!req.has_param("service")) {
        sendError(res, 400, ErrorCode::ValidationError,
                  "Missing query param: service");
        return;
    }
    UptimeResponse respBody;
    if (!getMonitoringClient().GetUptimeMonth(req.get_param_value("service"), &respBody,
                                              std::chrono::milliseconds(timeoutMs))) {
        sendError(res, 502, ErrorCode::InternalError,
                  "Monitoring service unavailable");
        return;
    }
    res.status = 200;
    res.set_content(json(respBody).dump(), "application/json");
}

void handleUptimeYear(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("MONITORING_HTTP_TIMEOUT_MS", 2000);
    if (!req.has_param("service")) {
        sendError(res, 400, ErrorCode::ValidationError,
                  "Missing query param: service");
        return;
    }
    UptimeResponse respBody;
    if (!getMonitoringClient().GetUptimeYear(req.get_param_value("service"), &respBody,
                                             std::chrono::milliseconds(timeoutMs))) {
        sendError(res, 502, ErrorCode::InternalError,
                  "Monitoring service unavailable");
        return;
    }
    res.status = 200;
    res.set_content(json(respBody).dump(), "application/json");
}

// GET /aggregation/watermark
void handleAggregationWatermark(const httplib::Request&, httplib::Response& res) {
    const int timeoutMs = getEnvInt("AGGREGATION_GRPC_TIMEOUT_MS", 2000);

    metricsys::aggregation::GetWatermarkRequest rpcReq;
    metricsys::aggregation::GetWatermarkResponse rpcResp;

    auto status = getAggregationClient().GetWatermark(
        rpcReq, &rpcResp, std::chrono::milliseconds(timeoutMs));
    if (!status.ok()) {
        sendError(res, 500, ErrorCode::InternalError,
                  "Aggregation service unavailable: " + status.error_message());
        return;
    }

    GetWatermarkResponse httpResp;
    if (rpcResp.has_last_aggregated_at()) {
        httpResp.last_aggregated_at = toIsoString(rpcResp.last_aggregated_at());
    }

    res.status = 200;
    res.set_content(json(httpResp).dump(), "application/json");
}

// POST /aggregation/page-views
void handleAggregationPageViews(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("AGGREGATION_GRPC_TIMEOUT_MS", 2000);

    try {
        auto body = json::parse(req.body);
        GetPageViewsAggRequest httpReq = body.get<GetPageViewsAggRequest>();

        metricsys::aggregation::GetPageViewsAggRequest rpcReq;
        rpcReq.set_project_id(httpReq.project_id);

        google::protobuf::Timestamp fromTs;
        google::protobuf::Timestamp toTs;
        if (!parseIsoTimestamp(httpReq.time_range.from, &fromTs) ||
            !parseIsoTimestamp(httpReq.time_range.to, &toTs)) {
            sendError(res, 400, ErrorCode::ValidationError,
                      "Invalid time_range format (expecting RFC3339 date-time)");
            return;
        }
        auto* tr = rpcReq.mutable_time_range();
        tr->mutable_from()->CopyFrom(fromTs);
        tr->mutable_to()->CopyFrom(toTs);

        if (httpReq.page.has_value()) {
            rpcReq.set_page(httpReq.page.value());
        }
        if (httpReq.pagination.has_value()) {
            auto* p = rpcReq.mutable_pagination();
            if (httpReq.pagination->limit.has_value()) {
                p->set_limit(httpReq.pagination->limit.value());
            }
            if (httpReq.pagination->offset.has_value()) {
                p->set_offset(httpReq.pagination->offset.value());
            }
        }

        metricsys::aggregation::GetPageViewsAggResponse rpcResp;
        auto status = getAggregationClient().GetPageViewsAgg(
            rpcReq, &rpcResp, std::chrono::milliseconds(timeoutMs));
        if (!status.ok()) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to fetch aggregation results: " + status.error_message());
            return;
        }

        GetPageViewsAggResponse httpResp;
        for (const auto& row : rpcResp.rows()) {
            AggPageViewsRow r;
            r.time_bucket = toIsoString(row.time_bucket());
            r.project_id = row.project_id();
            r.page = row.page();
            r.views_count = row.views_count();
            r.unique_users = row.unique_users();
            r.unique_sessions = row.unique_sessions();
            r.created_at = toIsoString(row.created_at());
            httpResp.rows.push_back(std::move(r));
        }

        res.status = 200;
        res.set_content(json(httpResp).dump(), "application/json");
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::ValidationError,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// POST /aggregation/clicks
void handleAggregationClicks(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("AGGREGATION_GRPC_TIMEOUT_MS", 2000);

    try {
        auto body = json::parse(req.body);
        GetClicksAggRequest httpReq = body.get<GetClicksAggRequest>();

        metricsys::aggregation::GetClicksAggRequest rpcReq;
        rpcReq.set_project_id(httpReq.project_id);

        google::protobuf::Timestamp fromTs;
        google::protobuf::Timestamp toTs;
        if (!parseIsoTimestamp(httpReq.time_range.from, &fromTs) ||
            !parseIsoTimestamp(httpReq.time_range.to, &toTs)) {
            sendError(res, 400, ErrorCode::ValidationError,
                      "Invalid time_range format (expecting RFC3339 date-time)");
            return;
        }
        auto* tr = rpcReq.mutable_time_range();
        tr->mutable_from()->CopyFrom(fromTs);
        tr->mutable_to()->CopyFrom(toTs);

        if (httpReq.page.has_value()) {
            rpcReq.set_page(httpReq.page.value());
        }
        if (httpReq.element_id.has_value()) {
            rpcReq.set_element_id(httpReq.element_id.value());
        }
        if (httpReq.pagination.has_value()) {
            auto* p = rpcReq.mutable_pagination();
            if (httpReq.pagination->limit.has_value()) {
                p->set_limit(httpReq.pagination->limit.value());
            }
            if (httpReq.pagination->offset.has_value()) {
                p->set_offset(httpReq.pagination->offset.value());
            }
        }

        metricsys::aggregation::GetClicksAggResponse rpcResp;
        auto status = getAggregationClient().GetClicksAgg(
            rpcReq, &rpcResp, std::chrono::milliseconds(timeoutMs));
        if (!status.ok()) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to fetch aggregation results: " + status.error_message());
            return;
        }

        GetClicksAggResponse httpResp;
        for (const auto& row : rpcResp.rows()) {
            AggClicksRow r;
            r.time_bucket = toIsoString(row.time_bucket());
            r.project_id = row.project_id();
            r.page = row.page();
            if (row.has_element_id()) {
                r.element_id = row.element_id();
            }
            r.clicks_count = row.clicks_count();
            r.unique_users = row.unique_users();
            r.unique_sessions = row.unique_sessions();
            r.created_at = toIsoString(row.created_at());
            httpResp.rows.push_back(std::move(r));
        }

        res.status = 200;
        res.set_content(json(httpResp).dump(), "application/json");
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::ValidationError,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// POST /aggregation/performance
void handleAggregationPerformance(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("AGGREGATION_GRPC_TIMEOUT_MS", 2000);

    try {
        auto body = json::parse(req.body);
        GetPerformanceAggRequest httpReq = body.get<GetPerformanceAggRequest>();

        metricsys::aggregation::GetPerformanceAggRequest rpcReq;
        rpcReq.set_project_id(httpReq.project_id);

        google::protobuf::Timestamp fromTs;
        google::protobuf::Timestamp toTs;
        if (!parseIsoTimestamp(httpReq.time_range.from, &fromTs) ||
            !parseIsoTimestamp(httpReq.time_range.to, &toTs)) {
            sendError(res, 400, ErrorCode::ValidationError,
                      "Invalid time_range format (expecting RFC3339 date-time)");
            return;
        }
        auto* tr = rpcReq.mutable_time_range();
        tr->mutable_from()->CopyFrom(fromTs);
        tr->mutable_to()->CopyFrom(toTs);

        if (httpReq.page.has_value()) {
            rpcReq.set_page(httpReq.page.value());
        }
        if (httpReq.pagination.has_value()) {
            auto* p = rpcReq.mutable_pagination();
            if (httpReq.pagination->limit.has_value()) {
                p->set_limit(httpReq.pagination->limit.value());
            }
            if (httpReq.pagination->offset.has_value()) {
                p->set_offset(httpReq.pagination->offset.value());
            }
        }

        metricsys::aggregation::GetPerformanceAggResponse rpcResp;
        auto status = getAggregationClient().GetPerformanceAgg(
            rpcReq, &rpcResp, std::chrono::milliseconds(timeoutMs));
        if (!status.ok()) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to fetch aggregation results: " + status.error_message());
            return;
        }

        GetPerformanceAggResponse httpResp;
        for (const auto& row : rpcResp.rows()) {
            AggPerformanceRow r;
            r.time_bucket = toIsoString(row.time_bucket());
            r.project_id = row.project_id();
            r.page = row.page();
            r.samples_count = row.samples_count();
            r.avg_total_load_ms = row.avg_total_load_ms();
            r.p95_total_load_ms = row.p95_total_load_ms();
            r.avg_ttfb_ms = row.avg_ttfb_ms();
            r.p95_ttfb_ms = row.p95_ttfb_ms();
            r.avg_fcp_ms = row.avg_fcp_ms();
            r.p95_fcp_ms = row.p95_fcp_ms();
            r.avg_lcp_ms = row.avg_lcp_ms();
            r.p95_lcp_ms = row.p95_lcp_ms();
            r.created_at = toIsoString(row.created_at());
            httpResp.rows.push_back(std::move(r));
        }

        res.status = 200;
        res.set_content(json(httpResp).dump(), "application/json");
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::ValidationError,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// POST /aggregation/errors
void handleAggregationErrors(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("AGGREGATION_GRPC_TIMEOUT_MS", 2000);

    try {
        auto body = json::parse(req.body);
        GetErrorsAggRequest httpReq = body.get<GetErrorsAggRequest>();

        metricsys::aggregation::GetErrorsAggRequest rpcReq;
        rpcReq.set_project_id(httpReq.project_id);

        google::protobuf::Timestamp fromTs;
        google::protobuf::Timestamp toTs;
        if (!parseIsoTimestamp(httpReq.time_range.from, &fromTs) ||
            !parseIsoTimestamp(httpReq.time_range.to, &toTs)) {
            sendError(res, 400, ErrorCode::ValidationError,
                      "Invalid time_range format (expecting RFC3339 date-time)");
            return;
        }
        auto* tr = rpcReq.mutable_time_range();
        tr->mutable_from()->CopyFrom(fromTs);
        tr->mutable_to()->CopyFrom(toTs);

        if (httpReq.page.has_value()) {
            rpcReq.set_page(httpReq.page.value());
        }
        if (httpReq.error_type.has_value()) {
            rpcReq.set_error_type(httpReq.error_type.value());
        }
        if (httpReq.pagination.has_value()) {
            auto* p = rpcReq.mutable_pagination();
            if (httpReq.pagination->limit.has_value()) {
                p->set_limit(httpReq.pagination->limit.value());
            }
            if (httpReq.pagination->offset.has_value()) {
                p->set_offset(httpReq.pagination->offset.value());
            }
        }

        metricsys::aggregation::GetErrorsAggResponse rpcResp;
        auto status = getAggregationClient().GetErrorsAgg(
            rpcReq, &rpcResp, std::chrono::milliseconds(timeoutMs));
        if (!status.ok()) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to fetch aggregation results: " + status.error_message());
            return;
        }

        GetErrorsAggResponse httpResp;
        for (const auto& row : rpcResp.rows()) {
            AggErrorsRow r;
            r.time_bucket = toIsoString(row.time_bucket());
            r.project_id = row.project_id();
            r.page = row.page();
            if (row.has_error_type()) {
                r.error_type = row.error_type();
            }
            r.errors_count = row.errors_count();
            r.warning_count = row.warning_count();
            r.critical_count = row.critical_count();
            r.unique_users = row.unique_users();
            r.created_at = toIsoString(row.created_at());
            httpResp.rows.push_back(std::move(r));
        }

        res.status = 200;
        res.set_content(json(httpResp).dump(), "application/json");
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::ValidationError,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// POST /aggregation/custom-events
void handleAggregationCustomEvents(const httplib::Request& req, httplib::Response& res) {
    const int timeoutMs = getEnvInt("AGGREGATION_GRPC_TIMEOUT_MS", 2000);

    try {
        auto body = json::parse(req.body);
        GetCustomEventsAggRequest httpReq = body.get<GetCustomEventsAggRequest>();

        metricsys::aggregation::GetCustomEventsAggRequest rpcReq;
        rpcReq.set_project_id(httpReq.project_id);
        rpcReq.set_event_name(httpReq.event_name);

        google::protobuf::Timestamp fromTs;
        google::protobuf::Timestamp toTs;
        if (!parseIsoTimestamp(httpReq.time_range.from, &fromTs) ||
            !parseIsoTimestamp(httpReq.time_range.to, &toTs)) {
            sendError(res, 400, ErrorCode::ValidationError,
                      "Invalid time_range format (expecting RFC3339 date-time)");
            return;
        }
        auto* tr = rpcReq.mutable_time_range();
        tr->mutable_from()->CopyFrom(fromTs);
        tr->mutable_to()->CopyFrom(toTs);

        if (httpReq.page.has_value()) {
            rpcReq.set_page(httpReq.page.value());
        }
        if (httpReq.pagination.has_value()) {
            auto* p = rpcReq.mutable_pagination();
            if (httpReq.pagination->limit.has_value()) {
                p->set_limit(httpReq.pagination->limit.value());
            }
            if (httpReq.pagination->offset.has_value()) {
                p->set_offset(httpReq.pagination->offset.value());
            }
        }

        metricsys::aggregation::GetCustomEventsAggResponse rpcResp;
        auto status = getAggregationClient().GetCustomEventsAgg(
            rpcReq, &rpcResp, std::chrono::milliseconds(timeoutMs));
        if (!status.ok()) {
            sendError(res, 500, ErrorCode::InternalError,
                      "Failed to fetch aggregation results: " + status.error_message());
            return;
        }

        GetCustomEventsAggResponse httpResp;
        for (const auto& row : rpcResp.rows()) {
            AggCustomEventsRow r;
            r.time_bucket = toIsoString(row.time_bucket());
            r.project_id = row.project_id();
            r.event_name = row.event_name();
            if (row.has_page()) {
                r.page = row.page();
            }
            r.events_count = row.events_count();
            r.unique_users = row.unique_users();
            r.unique_sessions = row.unique_sessions();
            r.created_at = toIsoString(row.created_at());
            httpResp.rows.push_back(std::move(r));
        }

        res.status = 200;
        res.set_content(json(httpResp).dump(), "application/json");
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::ValidationError,
                  std::string("Invalid JSON: ") + e.what());
    }
}

// ==================== Route Registration ====================

void registerRoutes(httplib::Server& server) {
    server.Get("/health/ping", handleHealthPing);
    server.Post("/page-views", handlePageView);
    server.Post("/clicks", handleClick);
    server.Post("/performance", handlePerformance);
    server.Post("/errors", handleErrorEvent);
    server.Post("/custom-events", handleCustomEvent);
    server.Get("/uptime", handleUptime);
    server.Get("/uptime/day", handleUptimeDay);
    server.Get("/uptime/week", handleUptimeWeek);
    server.Get("/uptime/month", handleUptimeMonth);
    server.Get("/uptime/year", handleUptimeYear);
    server.Get("/aggregation/watermark", handleAggregationWatermark);
    server.Post("/aggregation/page-views", handleAggregationPageViews);
    server.Post("/aggregation/clicks", handleAggregationClicks);
    server.Post("/aggregation/performance", handleAggregationPerformance);
    server.Post("/aggregation/errors", handleAggregationErrors);
    server.Post("/aggregation/custom-events", handleAggregationCustomEvents);
}
