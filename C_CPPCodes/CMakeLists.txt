cmake_minimum_required(VERSION 3.10)   # 声明cmake支持的最低版本

project(test_7zip VERSION 0.1.0)   # 项目信息

# set(CMAKE_CXX_STANDARD 14)	# 将 C++ 标准设置为 C++ 11， google至少 c++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall")


# 向环境中添加头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    # ${PROJECT_SOURCE_DIR}/include/googletest
    # ${PROJECT_SOURCE_DIR}/include/boost_1_84_0
    # ${PROJECT_SOURCE_DIR}/include/sqlite3
    # ${PROJECT_SOURCE_DIR}/include/spdlog
)

# 向环境中添加库文件搜索路径
link_directories(
    # ${PROJECT_SOURCE_DIR}/lib
    # ${PROJECT_SOURCE_DIR}/include/boost_1_84_0/lib
    # ${PROJECT_SOURCE_DIR}/include/googletest/lib
    # ${PROJECT_SOURCE_DIR}/include/sqlite3/lib
	${PROJECT_SOURCE_DIR}/include/bit7z/lib
)

# 源代码文件
set(
    SRC_FILES
    # ./src/main.cpp
    # ${PROJECT_SOURCE_DIR}/src/boost_demo.cpp
    # ${PROJECT_SOURCE_DIR}/src/gtest_demo.cpp
    # ${PROJECT_SOURCE_DIR}/src/WSqlite.cpp
    # ${PROJECT_SOURCE_DIR}/src/sqlite_demo.cpp
    # ${PROJECT_SOURCE_DIR}/src/runlog.cpp
    # ${PROJECT_SOURCE_DIR}/src/hrg_log.cpp
    # ${PROJECT_SOURCE_DIR}/src/json_demo.cpp
	${PROJECT_SOURCE_DIR}/src/test_code/test_7zip.cpp

)
# 生成测试可执行程序
# add_executable(${PROJECT_NAME} ./src/log_test/runlog.cpp)     # 指定生成目标

add_executable(${PROJECT_NAME}  ${SRC_FILES})

# 指定目标或其依赖项要使用的库或标志
# target_link_libraries(${PROJECT_NAME} pthread)
# target_link_libraries(${PROJECT_NAME} boost_thread-mgw13-mt-x64-1_84)
# target_link_libraries(${PROJECT_NAME} gtest gtest_main)
# target_link_libraries(${PROJECT_NAME} sqlite3)
target_link_libraries(${PROJECT_NAME} PRIVATE bit7z64)

# 设置执行文件，链接库文件输出路径
SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/out)
SET(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/out)

# set(SQL_DLL_DIR ${PROJECT_SOURCE_DIR}/include/sqlite3/lib)
# file(COPY ${SQL_DLL_DIR}/sqlite3.dll DESTINATION ${PROJECT_SOURCE_DIR}/out)
#============================================


# # 递归查找所有.cpp文件路径
# file(GLOB_RECURSE ALL_SRC
#     ${PROJECT_SOURCE_DIR}/*.cpp
# )
# # 过滤掉列表中目录
# list(FILTER ALL_SRC EXCLUDE REGEX ${PROJECT_SOURCE_DIR}/build/*)    


# message(STATUS "This is ALL_SRC:" ${ALL_SRC})

# add_executable(${CMAKE_PROJECT_NAME} ${ALL_SRC})     # 指定生成目标(可执行文件名 源码名)
#============================================
# set和unset，命令设置或者取消设置变量
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)  # 开启打包功能
# 设置执行文件，链接库文件输出路径
# SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/out)
# SET(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/out)

# 链接测试库,pthread 库一定要写在 ${GTEST_BOTH_LIBRARIES} 的后面, 否则编译时会报错,



# include(file|module) 命令将cmake文件或者模块加载
# include(CTest) 
# enable_testing()   # 开启测试相关功能，在根目录下的CMakeLists.txt中开启
# add_subdirectory(src)	# 添加模块路径
