cmake_minimum_required(VERSION 3.14)
project(api-service)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Совместимость с устаревшими CMake в зависимостях
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "")

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
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

# AMQP-CPP опции до FetchContent_MakeAvailable
set(AMQP-CPP_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(AMQP-CPP_LINUX_TCP ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    amqpcpp
    GIT_REPOSITORY https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
    GIT_TAG v4.3.26
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(httplib json amqpcpp)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(api-service ${SOURCES})
target_link_libraries(api-service PRIVATE httplib::httplib nlohmann_json::nlohmann_json amqpcpp)
