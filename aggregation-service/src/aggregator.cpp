#include "aggregator.h"

namespace aggregation {

Aggregator::Aggregator() {
    // TODO: Инициализация
}

Aggregator::~Aggregator() = default;

void Aggregator::run() {
    // TODO: Основной цикл агрегации
}

double Aggregator::calculateAverage(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& val : values) {
        sum += val;
    }
    return sum / static_cast<double>(values.size());
}

double Aggregator::calculateMin(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double min = values[0];
    for (const auto& val : values) {
        if (val < min) min = val;
    }
    return min;
}

double Aggregator::calculateMax(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double max = values[0];
    for (const auto& val : values) {
        if (val > max) max = val;
    }
    return max;
}

} // namespace aggregation

