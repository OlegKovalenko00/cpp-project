#!/bin/bash

set -e

echo "=== Building api-service with unit tests ==="
cd /home/arsenchik/programming/cppprojects/CPP_second_year/cpp-project/api-service/build

echo "Running CMake..."
cmake ..

echo "Building api_unit_tests..."
make -j$(nproc) api_unit_tests

echo ""
echo "=== Running Unit Tests ==="
./api_unit_tests

echo ""
echo "=== All tests completed ==="

