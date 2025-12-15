#pragma once

#include "models.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

// ==================== Handlers ====================

// GET /health/ping
void handleHealthPing(const httplib::Request& req, httplib::Response& res);

// POST /page-views
void handlePageView(const httplib::Request& req, httplib::Response& res);

// POST /clicks
void handleClick(const httplib::Request& req, httplib::Response& res);

// POST /performance
void handlePerformance(const httplib::Request& req, httplib::Response& res);

// POST /errors
void handleErrorEvent(const httplib::Request& req, httplib::Response& res);

// POST /custom-events
void handleCustomEvent(const httplib::Request& req, httplib::Response& res);

// GET /aggregation/watermark
void handleAggregationWatermark(const httplib::Request& req, httplib::Response& res);

// POST /aggregation/page-views
void handleAggregationPageViews(const httplib::Request& req, httplib::Response& res);

// POST /aggregation/clicks
void handleAggregationClicks(const httplib::Request& req, httplib::Response& res);

// POST /aggregation/performance
void handleAggregationPerformance(const httplib::Request& req, httplib::Response& res);

// POST /aggregation/errors
void handleAggregationErrors(const httplib::Request& req, httplib::Response& res);

// POST /aggregation/custom-events
void handleAggregationCustomEvents(const httplib::Request& req, httplib::Response& res);

// ==================== Route Registration ====================

void registerRoutes(httplib::Server& server);
