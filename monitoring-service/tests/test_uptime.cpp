#include <cassert>
#include <iostream>

#include "database.h"

namespace {
double percent(const PeriodStat& p) {
    if (p.total == 0) return 0.0;
    return static_cast<double>(p.ok) * 100.0 / static_cast<double>(p.total);
}
} 
int main() {
    {
        PeriodStat p{0, 0};
        assert(percent(p) == 0.0);
    }
    {
        PeriodStat p{10, 10};
        assert(percent(p) == 100.0);
    }
    {
        PeriodStat p{1, 2};
        assert(percent(p) == 50.0);
    }
    {
        UptimeStats stats{
            {5, 10},   
            {7, 10},   
            {0, 0},    
            {100, 200} 
        };
        assert(percent(stats.day) == 50.0);
        assert(percent(stats.week) == 70.0);
        assert(percent(stats.month) == 0.0);
        assert(percent(stats.year) == 50.0);
    }

    std::cout << "test_uptime: OK" << std::endl;
    return 0;
}
