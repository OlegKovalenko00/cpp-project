cmake_minimum_required(VERSION 3.14)
project(api-service)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    GIT_SHALLOW    TRUE
)

# rabbitmq-c - синхронный C клиент для RabbitMQ
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_SSL_SUPPORT OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    rabbitmq-c
    GIT_REPOSITORY https://github.com/alanxz/rabbitmq-c.git
    GIT_TAG        v0.14.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(httplib json rabbitmq-c)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(api-service ${SOURCES})
target_include_directories(api-service PRIVATE ${rabbitmq-c_SOURCE_DIR}/include)
target_link_libraries(api-service PRIVATE httplib::httplib nlohmann_json::nlohmann_json rabbitmq-static)
