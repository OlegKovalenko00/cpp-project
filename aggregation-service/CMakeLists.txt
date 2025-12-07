cmake_minimum_required(VERSION 3.10)
project(aggregation-service)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Основной исполняемый файл
file(GLOB SOURCES "src/*.cpp")
add_executable(aggregation-service ${SOURCES})

# Тесты
enable_testing()

add_executable(test_aggregator tests/test_aggregator.cpp src/aggregator.cpp)
target_include_directories(test_aggregator PRIVATE include)
add_test(NAME test_aggregator COMMAND test_aggregator)

add_executable(test_database tests/test_database.cpp)
target_include_directories(test_database PRIVATE include)
add_test(NAME test_database COMMAND test_database)

