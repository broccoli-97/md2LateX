add_library(md2LateX_lib
    md_converter.cpp
    # ...其他模块 cpp 文件
)

# 指定头文件所在目录
target_include_directories(md2LateX_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

target_link_libraries(md2LateX_lib PRIVATE CURL::libcurl)