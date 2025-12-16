#pragma once

#include "database.h"

#include <nlohmann/json.hpp>
#include <string>

void process_page_view(const nlohmann::json& data, const DatabaseConfig& db_config);
void process_click_event(const nlohmann::json& data, const DatabaseConfig& db_config);
void process_performance_event(const nlohmann::json& data, const DatabaseConfig& db_config);
void process_error_event(const nlohmann::json& data, const DatabaseConfig& db_config);
void process_custom_event(const nlohmann::json& data, const DatabaseConfig& db_config);

void process_message(const std::string& queue, const std::string& message, const DatabaseConfig& db_config);
