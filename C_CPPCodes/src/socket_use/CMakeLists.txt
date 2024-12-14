cmake_minimum_required(VERSION 3.16.3)
project(socket_use VERSION 0.1.0)

# 添加第三方库的头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include)

# 添加第三方库的链接库文件路径
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/lib)

# 生成可执行文件
add_executable(server_demo ${CMAKE_CURRENT_SOURCE_DIR}/server_demo.cpp)
add_executable(client_demo ${CMAKE_CURRENT_SOURCE_DIR}/client_demo.cpp)


if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows ")  
    # 将头文件路径加到与库形成对应关系
    target_link_libraries(server_demo wsock32 pthread)
    target_link_libraries(client_demo wsock32 pthread)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
    target_link_libraries(server_demo pthread)
    target_link_libraries(client_demo pthread)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin") 
    MESSAGE(STATUS "current platform: Mac OS X") 
ELSE () 
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}") 
endif (CMAKE_SYSTEM_NAME MATCHES "Linux") 
