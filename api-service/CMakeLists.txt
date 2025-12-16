cmake_minimum_required(VERSION 3.15)
project(api-service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GRPC REQUIRED grpc++ grpc)
pkg_check_modules(RABBITMQ REQUIRED librabbitmq)

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

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

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
FetchContent_MakeAvailable(libpqxx cpp_httplib json googletest)

# Исходные файлы api-service
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Добавляем уже сгенерированные protobuf файлы из src/
set(PROTO_SOURCES
    src/aggregation.pb.cc
    src/aggregation.grpc.pb.cc
)

add_executable(api-service
    ${SOURCES}
    ${PROTO_SOURCES}
)

set_target_properties(api-service PROPERTIES OUTPUT_NAME "api-service")

target_include_directories(api-service
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Protobuf_INCLUDE_DIRS}
        ${GRPC_INCLUDE_DIRS}
        ${cpp_httplib_SOURCE_DIR}
        ${json_SOURCE_DIR}/include
        ${RABBITMQ_INCLUDE_DIRS}
)

target_link_libraries(api-service
    PRIVATE
        ${GRPC_LIBRARIES}
        ${Protobuf_LIBRARIES}
        pqxx
        pq
        ${RABBITMQ_LIBRARIES}
        Threads::Threads
)

target_link_directories(api-service PRIVATE ${GRPC_LIBRARY_DIRS} ${RABBITMQ_LIBRARY_DIRS})

# ===== Unit Tests =====
enable_testing()

# Create a library with testable components (without main.cpp)
add_library(api_core
    src/handlers.cpp
    src/models.cpp
    src/rabbitmq.cpp
    src/aggregation_client.cpp
    src/monitoring_client.cpp
    src/aggregation.pb.cc
    src/aggregation.grpc.pb.cc
)

target_include_directories(api_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Protobuf_INCLUDE_DIRS}
    ${GRPC_INCLUDE_DIRS}
    ${cpp_httplib_SOURCE_DIR}
    ${json_SOURCE_DIR}/include
    ${RABBITMQ_INCLUDE_DIRS}
)

target_link_libraries(api_core PUBLIC
    ${GRPC_LIBRARIES}
    ${Protobuf_LIBRARIES}
    pqxx
    pq
    ${RABBITMQ_LIBRARIES}
    Threads::Threads
)

target_link_directories(api_core PUBLIC ${GRPC_LIBRARY_DIRS} ${RABBITMQ_LIBRARY_DIRS})

# Unit tests executable
add_executable(api_unit_tests
    tests/test_models_unit.cpp
    tests/test_handlers_unit.cpp
)

target_link_libraries(api_unit_tests
    PRIVATE
        api_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME ApiUnitTests COMMAND api_unit_tests)

