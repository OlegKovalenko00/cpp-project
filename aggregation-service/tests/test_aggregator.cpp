#include <iostream>
#include <cassert>
#include "aggregator.h"

void testCalculateAverage() {
    std::vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0};
    double result = aggregation::Aggregator::calculateAverage(values);
    assert(result == 3.0);
    std::cout << "testCalculateAverage passed" << std::endl;
}

void testCalculateMin() {
    std::vector<double> values = {5.0, 2.0, 8.0, 1.0, 9.0};
    double result = aggregation::Aggregator::calculateMin(values);
    assert(result == 1.0);
    std::cout << "testCalculateMin passed" << std::endl;
}

void testCalculateMax() {
    std::vector<double> values = {5.0, 2.0, 8.0, 1.0, 9.0};
    double result = aggregation::Aggregator::calculateMax(values);
    assert(result == 9.0);
    std::cout << "testCalculateMax passed" << std::endl;
}

int main() {
    testCalculateAverage();
    testCalculateMin();
    testCalculateMax();

    std::cout << "All aggregator tests passed!" << std::endl;
    return 0;
}

