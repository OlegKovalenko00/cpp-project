cmake_minimum_required(VERSION 3.10)
project(metrics_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(metrics_service
    src/main.cpp
    src/metrics.cpp
)

set_target_properties(metrics_service PROPERTIES OUTPUT_NAME "metrics-service")

target_include_directories(metrics_service
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
