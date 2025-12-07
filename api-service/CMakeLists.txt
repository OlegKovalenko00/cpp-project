cmake_minimum_required(VERSION 3.10)
project(api-service)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)
include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(httplib json)



file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(api-service ${SOURCES})
target_link_libraries(api-service PRIVATE httplib::httplib nlohmann_json::nlohmann_json)
