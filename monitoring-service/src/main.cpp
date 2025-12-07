#include <iostream>
#include "monitor.h"

int main() {
    std::cout << "Monitoring service started" << std::endl;
    run_monitoring_loop();
    return 0;
}