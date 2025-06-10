add_executable(md2LateX main.cpp)
target_link_libraries(md2LateX PRIVATE md2LateX_lib CURL::libcurl nlohmann_json::nlohmann_json)