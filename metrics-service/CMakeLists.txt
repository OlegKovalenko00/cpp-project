cmake_minimum_required(VERSION 3.15)
project(metrics_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ===================== gRPC и Protobuf - системные =====================
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++ grpc)

# ===================== libpqxx через FetchContent =====================
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.9.2
    GIT_SHALLOW TRUE
)
set(PQXX_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(PQXX_BUILD_DOC OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libpqxx)

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/metrics.cpp
    src/database.cpp
    src/metrics.pb.cc
    src/metrics.grpc.pb.cc
)

add_executable(metrics_service ${SOURCES})

set_target_properties(metrics_service PROPERTIES OUTPUT_NAME "metrics-service")

target_include_directories(metrics_service
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Protobuf_INCLUDE_DIRS}
        ${GRPC_INCLUDE_DIRS}
)

target_link_libraries(metrics_service
    PRIVATE
        ${GRPC_LIBRARIES}
        ${Protobuf_LIBRARIES}
        pqxx
        pq
        Threads::Threads
)

target_link_directories(metrics_service PRIVATE ${GRPC_LIBRARY_DIRS})
