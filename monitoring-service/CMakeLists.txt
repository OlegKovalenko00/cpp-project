cmake_minimum_required(VERSION 3.10)
project(monitoring-service)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(monitoring-service ${SOURCES})
