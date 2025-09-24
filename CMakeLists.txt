cmake_minimum_required(VERSION 3.10)

project(live_recording)

set(CMAKE_CXX_STANDARD 17)


add_executable(${PROJECT_NAME} 
    main.cpp
    MainManager.cpp
    MassageQueue.cpp
    ApiUtils.cpp
    Recoder.cpp
)

add_compile_options(/utf-8 /execution-charset:utf-8)


find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
target_link_libraries(live_recording PRIVATE CURL::libcurl)
target_link_libraries(live_recording PRIVATE nlohmann_json::nlohmann_json)

# 将url.txt移动到生成的文件夹中
add_custom_command(TARGET live_recording POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/url.txt ${CMAKE_CURRENT_BINARY_DIR}/DEBUG/url.txt
)