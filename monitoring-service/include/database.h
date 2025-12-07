#pragma once
#include <string>

void db_init();
void db_write_result(const std::string& service_name, const std::string& url, bool ok);