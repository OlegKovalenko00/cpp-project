#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <vector>
#include <string>

namespace aggregation {

class Database;

class Aggregator {
public:
    explicit Aggregator(Database& db);
    ~Aggregator();

    void run();

    static double calculateAverage(const std::vector<double>& values);
    static double calculateMin(const std::vector<double>& values);
    static double calculateMax(const std::vector<double>& values);
private:
    Database& database_;
};

} // namespace aggregation

#endif // AGGREGATOR_H

