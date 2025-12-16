#include <cassert>
#include <iostream>

#include "database.h"

int main() {
    assert(db_is_ready() == false);

    std::string error;
    auto stats = db_get_uptime_stats("dummy-service", error);
    assert(!stats.has_value());
    assert(error == "DB not connected");

    try {
        db_write_result("dummy-service", "http://localhost/health/ping", true);
    } catch (...) {
        assert(false && "db_write_result threw unexpectedly");
    }

    std::cout << "test_database: OK (no DB connection present)" << std::endl;
    return 0;
}
