cmake_minimum_required(VERSION 3.10)
project(OptimizedSocket)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 查找nlohmann/json库
# find_package(nlohmann_json 3.2.0 QUIET)
# if(NOT nlohmann_json_FOUND)
#     # 如果找不到系统安装的json库，尝试使用FetchContent
#     include(FetchContent)
#     FetchContent_Declare(
#         json
#         URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
#     )
#     FetchContent_MakeAvailable(json)
#     set(JSON_LIBRARY nlohmann_json::nlohmann_json)
# else()
#     set(JSON_LIBRARY nlohmann_json::nlohmann_json)
# endif()

# 添加可执行文件
add_executable(optimized_socket_demo
    optimized_demo.cpp
    optimized_server.cpp
    optimized_client.cpp
    optimized_server.h
    optimized_client.h
    
)

target_include_directories(optimized_socket_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接库
if(WIN32)
    target_link_libraries(optimized_socket_demo ws2_32)
else()
    target_link_libraries(optimized_socket_demo pthread)
endif()

# 设置编译选项
if(MSVC)
    target_compile_options(optimized_socket_demo PRIVATE /W4)
    # 禁用一些MSVC警告
    target_compile_definitions(optimized_socket_demo PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(optimized_socket_demo PRIVATE -Wall -Wextra -O2)
endif()

# 添加头文件目录
target_include_directories(optimized_socket_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 设置调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_compile_options(optimized_socket_demo PRIVATE /Zi)
    else()
        target_compile_options(optimized_socket_demo PRIVATE -g)
    endif()
endif()

# 安装规则
install(TARGETS optimized_socket_demo
    RUNTIME DESTINATION bin
)

# 打印配置信息
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Using local nlohmann/json header") 