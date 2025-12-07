#pragma once
#include <string>
#include <httplib.h>

bool check_health(const std::string& url, std::string& message);