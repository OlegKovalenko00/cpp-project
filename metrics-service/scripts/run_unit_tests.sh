#!/bin/bash

set -e

echo "=== Building metrics-service with unit tests ==="
cd /home/arsenchik/programming/cppprojects/CPP_second_year/cpp-project/metrics-service/build

echo "Running CMake..."
cmake ..

echo "Building metrics_unit_tests..."
make -j$(nproc) metrics_unit_tests

echo ""
echo "=== Running Unit Tests ==="
./metrics_unit_tests

echo ""
echo "=== All tests completed ==="

