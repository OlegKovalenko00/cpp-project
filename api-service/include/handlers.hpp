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

// ==================== Route Registration ====================

void registerRoutes(httplib::Server& server);
