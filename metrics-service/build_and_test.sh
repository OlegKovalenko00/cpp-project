#!/bin/bash

# Скрипт для сборки и запуска юнит-тестов metrics-service

set -e

echo "=== Сборка юнит-тестов metrics-service ==="

# Переходим в директорию проекта
cd /home/arsenchik/programming/cppprojects/CPP_second_year/cpp-project/metrics-service

# Создаем build директорию если её нет
mkdir -p build
cd build

echo "Шаг 1: Запуск CMake..."
cmake .. || { echo "CMake failed!"; exit 1; }

echo ""
echo "Шаг 2: Сборка metrics_unit_tests..."
make metrics_unit_tests -j$(nproc) || { echo "Build failed!"; exit 1; }

echo ""
echo "Шаг 3: Проверка что бинарник создан..."
if [ -f "./metrics_unit_tests" ]; then
    echo "✓ metrics_unit_tests успешно собран"
    ls -lh ./metrics_unit_tests
else
    echo "✗ Файл metrics_unit_tests не найден"
    exit 1
fi

echo ""
echo "=== Запуск тестов ==="
./metrics_unit_tests

echo ""
echo "=== Тесты завершены ==="

