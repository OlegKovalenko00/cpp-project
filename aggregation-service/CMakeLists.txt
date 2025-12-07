cmake_minimum_required(VERSION 3.16)
project(aggregation-service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PostgreSQL REQUIRED)

include_directories(include)

file(GLOB CORE_SOURCES "src/aggregator.cpp" "src/database.cpp")
add_library(aggregation-core ${CORE_SOURCES})
target_include_directories(aggregation-core PUBLIC include)

target_link_libraries(aggregation-core
        PUBLIC
        PostgreSQL::PostgreSQL
)

add_executable(aggregation-service src/main.cpp)
target_link_libraries(aggregation-service PRIVATE aggregation-core)

enable_testing()

add_executable(test_aggregator tests/test_aggregator.cpp)
target_link_libraries(test_aggregator PRIVATE aggregation-core)
target_include_directories(test_aggregator PRIVATE include)

add_executable(test_database tests/test_database.cpp)
target_link_libraries(test_database PRIVATE aggregation-core)
target_include_directories(test_database PRIVATE include)
