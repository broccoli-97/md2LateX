cmake_minimum_required(VERSION 3.15)
project(md2LateX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_subdirectory(src)