cmake_minimum_required(VERSION 3.16)
project(aggregation-service)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)

# Используем общий proto из корня проекта
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(PROTO_FILE ${PROTO_DIR}/metrics.proto)

set(PROTO_SRCS ${GENERATED_DIR}/metrics.pb.cc)
set(PROTO_HDRS ${GENERATED_DIR}/metrics.pb.h)
set(GRPC_SRCS ${GENERATED_DIR}/metrics.grpc.pb.cc)
set(GRPC_HDRS ${GENERATED_DIR}/metrics.grpc.pb.h)

add_custom_command(
        OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
        COMMAND protobuf::protoc
        ARGS --proto_path=${PROTO_DIR}
        --cpp_out=${GENERATED_DIR}
        --grpc_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating protobuf and gRPC stubs from metrics.proto"
)

add_library(aggregation-core
        src/aggregator.cpp
        src/database.cpp
        src/metrics_client.cpp
        ${PROTO_SRCS}
        ${GRPC_SRCS}
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
