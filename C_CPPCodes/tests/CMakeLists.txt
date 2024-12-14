cmake_minimum_required(VERSION 3.16.3)
project(test_gtest VERSION 0.1.0)

set(OUTPUT_NAME test_gtest)
# 将 gtest 添加到项目中
# 关闭 gmock 功能
set(BUILD_GMOCK OFF)
link_directories(${CMAKE_SOURCE_DIR}/include/googletest/lib)

add_executable(${OUTPUT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/test_gtest.cpp)

target_include_directories(${OUTPUT_NAME} PUBLIC ${PROJECT_ROOT}/include/googletest)


target_link_libraries(${OUTPUT_NAME} PUBLIC gtest gtest_main)

add_test(NAME ${OUTPUT_NAME}
         COMMAND ${OUTPUT_NAME})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows ")  
    # 将头文件路径加到与库形成对应关系
    # target_link_libraries(${OUTPUT_NAME} wsock32 pthread)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
    # target_link_libraries(${OUTPUT_NAME} pthread)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin") 
    MESSAGE(STATUS "current platform: Mac OS X") 
ELSE () 
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}") 
endif (CMAKE_SYSTEM_NAME MATCHES "Linux") 
