#include "handlers.hpp"

#include <iostream>

#include "rabbitmq.hpp"

using json = nlohmann::json;

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

        getRabbitMQ().async_publish("", "page_views", json(event).dump());
        std::cout << "[PageView] Published event for page=" << event.page << std::endl;
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

        getRabbitMQ().async_publish("", "clicks", json(event).dump());
        std::cout << "[Click] Published event for page=" << event.page << " element_id=" << event.element_id << std::endl;
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

        getRabbitMQ().async_publish("", "performance_events", json(event).dump());
        std::cout << "[Performance] Published event for page=" << event.page << std::endl;
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

        getRabbitMQ().async_publish("", "error_events", json(event).dump());
        std::cout << "[ErrorEvent] Published event for page=" << event.page << " error_type=" << event.error_type << std::endl;
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

        getRabbitMQ().async_publish("", "custom_events", json(event).dump());
        std::cout << "[CustomEvent] Published event name=" << event.name << std::endl;
        sendAccepted(res);
    } catch (const json::exception& e) {
        sendError(res, 400, ErrorCode::InvalidCustomEvent,
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
}
