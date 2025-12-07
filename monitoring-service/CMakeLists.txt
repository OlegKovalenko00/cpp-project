cmake_minimum_required(VERSION 3.10)
project(monitoring-service)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)
include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
)
FetchContent_MakeAvailable(httplib)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(monitoring-service ${SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

target_include_directories(monitoring-service PRIVATE ${PQXX_INCLUDE_DIRS})

find_library(PQXX_LIBRARY NAMES pqxx libpqxx PATHS ${PQXX_LIBRARY_DIRS} /opt/homebrew/lib /usr/local/lib)
find_library(PQ_LIBRARY NAMES pq PATHS /opt/homebrew/opt/libpq/lib /opt/homebrew/lib /usr/local/lib)

if(PQXX_LIBRARY)
    target_link_libraries(monitoring-service PRIVATE ${PQXX_LIBRARY})
endif()

if(PQ_LIBRARY)
    target_link_libraries(monitoring-service PRIVATE ${PQ_LIBRARY})
endif()

target_link_libraries(monitoring-service PRIVATE httplib::httplib)

add_custom_target(run
    COMMAND $<TARGET_FILE:monitoring-service>
    DEPENDS monitoring-service
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
