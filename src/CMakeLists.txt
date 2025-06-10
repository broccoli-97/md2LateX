
# # 添加可执行文件
# add_executable(my_app main.cpp md_converter.cpp)

# # 链接库
# if(TARGET CURL::libcurl)
#     target_link_libraries(my_app PRIVATE CURL::libcurl)
# elseif(TARGET CURL::CURL)
#     target_link_libraries(my_app PRIVATE CURL::CURL)
# else()
#     target_link_libraries(my_app PRIVATE ${CURL_LIBRARIES})
#     target_include_directories(my_app PRIVATE ${CURL_INCLUDE_DIRS})
# endif()

# target_link_libraries(my_app PRIVATE nlohmann_json::nlohmann_json)

add_subdirectory(lib)
add_subdirectory(app)