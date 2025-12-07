#pragma once
#include <string>

// Simple logging helpers; currently write to stdout/stderr.
// Can be extended later to log into files or databases.
void log_info(const std::string& message);
void log_error(const std::string& message);
