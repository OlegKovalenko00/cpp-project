cmake_minimum_required(VERSION 3.16)
project(aggregation-service)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# httplib (header-only)
include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(httplib json)

find_package(OpenSSL REQUIRED)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)

# Используем общий proto из корня проекта
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

# Metrics proto
set(METRICS_PROTO_FILE ${PROTO_DIR}/metrics.proto)
set(METRICS_PROTO_SRCS ${GENERATED_DIR}/metrics.pb.cc)
set(METRICS_PROTO_HDRS ${GENERATED_DIR}/metrics.pb.h)
set(METRICS_GRPC_SRCS ${GENERATED_DIR}/metrics.grpc.pb.cc)
set(METRICS_GRPC_HDRS ${GENERATED_DIR}/metrics.grpc.pb.h)

# Aggregation proto
set(AGGREGATION_PROTO_FILE ${PROTO_DIR}/aggregation.proto)
set(AGGREGATION_PROTO_SRCS ${GENERATED_DIR}/aggregation.pb.cc)
set(AGGREGATION_PROTO_HDRS ${GENERATED_DIR}/aggregation.pb.h)
set(AGGREGATION_GRPC_SRCS ${GENERATED_DIR}/aggregation.grpc.pb.cc)
set(AGGREGATION_GRPC_HDRS ${GENERATED_DIR}/aggregation.grpc.pb.h)

# Generate metrics.proto
add_custom_command(
        OUTPUT ${METRICS_PROTO_SRCS} ${METRICS_PROTO_HDRS} ${METRICS_GRPC_SRCS} ${METRICS_GRPC_HDRS}
        COMMAND protobuf::protoc
        ARGS --proto_path=${PROTO_DIR}
        --cpp_out=${GENERATED_DIR}
        --grpc_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        ${METRICS_PROTO_FILE}
        DEPENDS ${METRICS_PROTO_FILE}
        COMMENT "Generating protobuf and gRPC stubs from metrics.proto"
)

# Generate aggregation.proto
add_custom_command(
        OUTPUT ${AGGREGATION_PROTO_SRCS} ${AGGREGATION_PROTO_HDRS} ${AGGREGATION_GRPC_SRCS} ${AGGREGATION_GRPC_HDRS}
        COMMAND protobuf::protoc
        ARGS --proto_path=${PROTO_DIR}
        --cpp_out=${GENERATED_DIR}
        --grpc_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        ${AGGREGATION_PROTO_FILE}
        DEPENDS ${AGGREGATION_PROTO_FILE}
        COMMENT "Generating protobuf and gRPC stubs from aggregation.proto"
)

add_library(aggregation-core
        src/aggregator.cpp
        src/database.cpp
        src/metrics_client.cpp
        src/handlers.cpp
        src/aggregation_server.cpp
        ${METRICS_PROTO_SRCS}
        ${METRICS_GRPC_SRCS}
        ${AGGREGATION_PROTO_SRCS}
        ${AGGREGATION_GRPC_SRCS}
)

target_include_directories(aggregation-core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBPQ_INCLUDE_DIRS}
        ${GENERATED_DIR}
)

target_link_libraries(aggregation-core PUBLIC
        ${LIBPQ_LIBRARIES}
        protobuf::libprotobuf
        gRPC::grpc++
        gRPC::grpc++_reflection
        httplib::httplib
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
)

add_executable(aggregation-service src/main.cpp)
target_link_libraries(aggregation-service PRIVATE aggregation-core)

enable_testing()

add_executable(test_aggregator tests/test_aggregator.cpp)
target_link_libraries(test_aggregator PRIVATE aggregation-core)
add_test(NAME test_aggregator COMMAND test_aggregator)

add_executable(test_database tests/test_database.cpp)
target_link_libraries(test_database PRIVATE aggregation-core)
add_test(NAME test_database COMMAND test_database)

add_executable(test_grpc_connection tests/test_grpc_connection.cpp)
target_link_libraries(test_grpc_connection PRIVATE aggregation-core)

add_executable(test_aggregation_grpc_server tests/test_aggregation_grpc_server.cpp)
target_link_libraries(test_aggregation_grpc_server PRIVATE aggregation-core)

