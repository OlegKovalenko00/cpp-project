cmake_minimum_required(VERSION 3.14)
project(monitoring-service)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_Declare(
    json
    URL            https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(httplib json)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(monitoring-service ${SOURCES})

# PostgreSQL and libpqxx - кроссплатформенный поиск
find_package(PkgConfig REQUIRED)

# Попытка найти через pkg-config
pkg_check_modules(PQXX libpqxx)
pkg_check_modules(LIBPQ libpq)

if(PQXX_FOUND)
    target_include_directories(monitoring-service PRIVATE ${PQXX_INCLUDE_DIRS})
    target_link_directories(monitoring-service PRIVATE ${PQXX_LIBRARY_DIRS})
endif()

if(LIBPQ_FOUND)
    target_include_directories(monitoring-service PRIVATE ${LIBPQ_INCLUDE_DIRS})
    target_link_directories(monitoring-service PRIVATE ${LIBPQ_LIBRARY_DIRS})
endif()

# Платформо-специфичные пути
if(APPLE)
    # macOS с Homebrew
    list(APPEND CMAKE_PREFIX_PATH 
        /opt/homebrew
        /opt/homebrew/opt/libpq
        /opt/homebrew/opt/libpqxx
        /usr/local
        /usr/local/opt/libpq
        /usr/local/opt/libpqxx
    )
    target_include_directories(monitoring-service PRIVATE
        /opt/homebrew/include
        /opt/homebrew/opt/libpq/include
        /usr/local/include
    )
endif()

# Поиск библиотек
find_library(PQXX_LIBRARY 
    NAMES pqxx libpqxx
    PATHS 
        ${PQXX_LIBRARY_DIRS}
        /opt/homebrew/lib
        /opt/homebrew/opt/libpqxx/lib
        /usr/local/lib
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
)

find_library(PQ_LIBRARY 
    NAMES pq libpq
    PATHS 
        ${LIBPQ_LIBRARY_DIRS}
        /opt/homebrew/lib
        /opt/homebrew/opt/libpq/lib
        /usr/local/lib
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
)

if(PQXX_LIBRARY)
    message(STATUS "Found libpqxx: ${PQXX_LIBRARY}")
    target_link_libraries(monitoring-service PRIVATE ${PQXX_LIBRARY})
else()
    message(FATAL_ERROR "libpqxx not found")
endif()

if(PQ_LIBRARY)
    message(STATUS "Found libpq: ${PQ_LIBRARY}")
    target_link_libraries(monitoring-service PRIVATE ${PQ_LIBRARY})
else()
    message(FATAL_ERROR "libpq not found")
endif()

target_link_libraries(monitoring-service PRIVATE httplib::httplib nlohmann_json::nlohmann_json)

add_custom_target(run
    COMMAND $<TARGET_FILE:monitoring-service>
    DEPENDS monitoring-service
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
