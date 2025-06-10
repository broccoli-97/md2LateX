add_library(md2LateX_lib
    md_converter.cpp
)

target_include_directories(md2LateX_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

target_link_libraries(md2LateX_lib PRIVATE CURL::libcurl)