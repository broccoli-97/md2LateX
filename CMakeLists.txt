cmake_minimum_required(VERSION 3.15)
project(md2LateX VERSION 1.0.0 LANGUAGES CXX)

# 设置C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # 设置vcpkg工具链文件路径 (修改为您自己的vcpkg路径)
# set(CMAKE_TOOLCHAIN_FILE "C:/Users/liuan/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# 查找需要的库
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# 添加include目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_subdirectory(src)