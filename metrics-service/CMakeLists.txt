cmake_minimum_required(VERSION 3.15)
project(metrics_service LANGUAGES CXX)

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

set(METRICS_CORE_SOURCES
    src/metrics.cpp
    src/database.cpp
    src/http_handler.cpp
    src/message_processor.cpp
    src/metrics.pb.cc
    src/metrics.grpc.pb.cc
)

add_library(metrics_service_core STATIC ${METRICS_CORE_SOURCES})

target_include_directories(metrics_service_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Protobuf_INCLUDE_DIRS}
        ${GRPC_INCLUDE_DIRS}
        ${cpp_httplib_SOURCE_DIR}
        ${json_SOURCE_DIR}/include
        ${RABBITMQ_INCLUDE_DIRS}
)

target_link_libraries(metrics_service_core
    PUBLIC
        ${GRPC_LIBRARIES}
        ${Protobuf_LIBRARIES}
        pqxx
        pq
        ${RABBITMQ_LIBRARIES}
        Threads::Threads
)

target_link_directories(metrics_service_core PUBLIC ${GRPC_LIBRARY_DIRS} ${RABBITMQ_LIBRARY_DIRS})

add_executable(metrics-service src/main.cpp src/rabbitmq.cpp)
set_target_properties(metrics-service PROPERTIES OUTPUT_NAME "metrics-service")
target_link_libraries(metrics-service PRIVATE metrics_service_core)

option(METRICS_BUILD_TESTS "Build metrics-service tests" ON)
if(METRICS_BUILD_TESTS)
    include(CTest)
    enable_testing()

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()
