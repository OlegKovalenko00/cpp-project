#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <vector>
#include <string>

namespace aggregation {

class Aggregator {
public:
    Aggregator();
    ~Aggregator();

    void run();

    static double calculateAverage(const std::vector<double>& values);
    static double calculateMin(const std::vector<double>& values);
    static double calculateMax(const std::vector<double>& values);
};

} // namespace aggregation

#endif // AGGREGATOR_H

