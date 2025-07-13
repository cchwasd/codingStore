cmake_minimum_required(VERSION 3.10)
project(MultiThreadSocket)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# 添加可执行文件
add_executable(socket_test
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mulThreadServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mulThreadClient.cpp
)
# 添加头文件
target_sources(socket_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mulThreadServer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mulThreadClient.h
)

# 链接Windows Socket库
if(WIN32)
    target_link_libraries(socket_test ws2_32)
endif()

# 设置编译选项
if(MSVC)
    target_compile_options(socket_test PRIVATE /W4)
else()
    target_compile_options(socket_test PRIVATE -Wall -Wextra)
endif()



# 添加头文件目录
target_include_directories(socket_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) 