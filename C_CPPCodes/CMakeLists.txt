cmake_minimum_required(VERSION 3.16.3)
project(BuildExample VERSION 0.1.0)

enable_testing()

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-std=c++14)
endif(CMAKE_COMPILER_IS_GNUCXX)

option(BUILD_DEBUG "Builds debug!!!" OFF)
if(BUILD_DEBUG)
    add_compile_options(-g -Wall -Wextra)
endif(BUILD_DEBUG)


# 设置执行文件，链接库文件输出路径
SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})


# 源代码文件
# set(SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)

# 添加第三方库的头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include)
# 添加第三方库的库文件路径
link_directories(${CMAKE_SOURCE_DIR}/include/lib)

# 添加子模块
# add_subdirectory(src/socket_use)

# # 生成可执行文件
# set(OUTPUT_NAME main)
# add_executable(${OUTPUT_NAME} ${SRC_FILES})

# 指定目标的依赖库
# target_link_libraries(${OUTPUT_NAME} PUBLIC pthread)

# 设置 编译测试选项
option(BUILD_TEST "Builds test!!!" ON)
if(BUILD_TEST)
    add_subdirectory(tests)
endif()