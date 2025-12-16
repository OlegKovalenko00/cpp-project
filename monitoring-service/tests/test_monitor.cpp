#include <cassert>

#include "logging.h"

int main() {
    log_debug("test debug");
    log_info("test info");
    log_warning("test warning");
    log_error("test error");
    return 0;
}