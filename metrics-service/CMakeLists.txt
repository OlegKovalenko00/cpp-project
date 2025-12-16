cmake_minimum_required(VERSION 3.15)
project(metrics_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Google Test for unit tests
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
pkg_check_modules(GRPC REQUIRED grpc++ grpc)
pkg_check_modules(RABBITMQ REQUIRED librabbitmq)

# Find protoc and grpc plugin
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
if(NOT GRPC_CPP_PLUGIN)
    message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()

# Generate protobuf files
set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../proto")
set(GENERATED_PROTOBUF_PATH "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

set(METRICS_PROTO "${PROTO_PATH}/metrics.proto")
set(METRICS_PB_CPP "${GENERATED_PROTOBUF_PATH}/metrics.pb.cc")
set(METRICS_PB_H "${GENERATED_PROTOBUF_PATH}/metrics.pb.h")
set(METRICS_GRPC_PB_CPP "${GENERATED_PROTOBUF_PATH}/metrics.grpc.pb.cc")
set(METRICS_GRPC_PB_H "${GENERATED_PROTOBUF_PATH}/metrics.grpc.pb.h")

add_custom_command(
    OUTPUT "${METRICS_PB_CPP}" "${METRICS_PB_H}"
           "${METRICS_GRPC_PB_CPP}" "${METRICS_GRPC_PB_H}"
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --grpc_out "${GENERATED_PROTOBUF_PATH}"
         --cpp_out "${GENERATED_PROTOBUF_PATH}"
         -I "${PROTO_PATH}"
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
         "${METRICS_PROTO}"
    DEPENDS "${METRICS_PROTO}"
    COMMENT "Generating protobuf files from metrics.proto"
)

set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.9.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_Declare(
    cpp_httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(PQXX_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(PQXX_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(libpqxx cpp_httplib json)

set(SOURCES
    src/main.cpp
    src/metrics.cpp
    src/database.cpp
    src/rabbitmq.cpp
    src/http_handler.cpp
    ${METRICS_PB_CPP}
    ${METRICS_GRPC_PB_CPP}
)

add_executable(metrics_service ${SOURCES})

set_target_properties(metrics_service PROPERTIES OUTPUT_NAME "metrics-service")

target_include_directories(metrics_service
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GENERATED_PROTOBUF_PATH}
        ${Protobuf_INCLUDE_DIRS}
        ${GRPC_INCLUDE_DIRS}
        ${cpp_httplib_SOURCE_DIR}
        ${json_SOURCE_DIR}/include
        ${RABBITMQ_INCLUDE_DIRS}
)

target_link_libraries(metrics_service
    PRIVATE
        ${GRPC_LIBRARIES}
        ${Protobuf_LIBRARIES}
        pqxx
        pq
        ${RABBITMQ_LIBRARIES}
        Threads::Threads
)

target_link_directories(metrics_service PRIVATE ${GRPC_LIBRARY_DIRS} ${RABBITMQ_LIBRARY_DIRS})

# ===== Unit Tests =====
enable_testing()

# Create a library with testable components (without main.cpp)
add_library(metrics_core
    src/metrics.cpp
    src/database.cpp
    src/http_handler.cpp
    ${METRICS_PB_CPP}
    ${METRICS_GRPC_PB_CPP}
)

target_include_directories(metrics_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GENERATED_PROTOBUF_PATH}
    ${Protobuf_INCLUDE_DIRS}
    ${GRPC_INCLUDE_DIRS}
    ${cpp_httplib_SOURCE_DIR}
    ${json_SOURCE_DIR}/include
    ${RABBITMQ_INCLUDE_DIRS}
)

target_link_libraries(metrics_core PUBLIC
    ${GRPC_LIBRARIES}
    ${Protobuf_LIBRARIES}
    pqxx
    pq
    ${RABBITMQ_LIBRARIES}
    Threads::Threads
)

target_link_directories(metrics_core PUBLIC ${GRPC_LIBRARY_DIRS} ${RABBITMQ_LIBRARY_DIRS})

# Unit tests executable
add_executable(metrics_unit_tests
    tests/test_database_unit.cpp
    tests/test_metrics_unit.cpp
)

target_link_libraries(metrics_unit_tests
    PRIVATE
        metrics_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME MetricsUnitTests COMMAND metrics_unit_tests)

