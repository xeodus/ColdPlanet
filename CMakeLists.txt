cmake_minimum_required(VERSION 3.16)
project(ColdPlanet C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB SRC_FILES "src/*.c")
add_executable(splash ${SRC_FILES})

target_include_directories(splash PRIVATE "${PROJECT_SOURCE_DIR}/src")
