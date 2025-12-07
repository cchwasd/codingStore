cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME SqlBase)
project(${PROJECT_NAME} VERSION 0.1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 设置编译选项
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
# 启动调试信息
set(CMAKE_BUILD_TYPE Debug)

# 设置执行文件，链接库文件输出路径
# SET(EXECUTABLE_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/bin)
# SET(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/bin)


# 指定mysql 路径
set(MYSQL_ROOT_DIR "C:/MPrograms/Database/mysql-8.3.0")

# 源代码文件
set(SRC_FILES 
    # ${CMAKE_CURRENT_SOURCE_DIR}/MainTest.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/SqliteOperat.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/SqliteOperat.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/mysql_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SqlBase.cpp
)

# 收集当前目录下所有的.cpp文件
# file(GLOB ALL_CPP_FILES *.cpp)
# file(GLOB ALL_H_FILES *.h)
# set(SRC_FILES ${ALL_CPP_FILES} ${ALL_H_FILES})

# 打印所有.cpp文件的名字
# foreach(file ${ALL_CPP_FILES})
#   message(STATUS "Found .cpp file: ${file}")
# endforeach()


# 添加第三方库的头文件路径
include_directories(
    # ${CMAKE_SOURCE_DIR}/include/sqlite3
    ${MYSQL_ROOT_DIR}/include
)
# 利用 add_library 创建一个名为database的target库
# add_library(database STATIC ${SRC_FILES})
# add_library(database SHARED ${SRC_FILES})



# 生成可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 链接库文件路径
target_link_directories(${PROJECT_NAME} PRIVATE
    # ${CMAKE_SOURCE_DIR}/lib/sqlite3
    ${MYSQL_ROOT_DIR}/lib
)

# 指定目标的依赖库
target_link_libraries(${PROJECT_NAME} PRIVATE
    # sqlite3      # 链接sqlite3库
    libmysql  # 链接mysql库
)

# Windows 额外依赖项
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 crypt32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${MYSQL_ROOT_DIR}/lib/libmysql.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying libmysql.dll to executable directory"
    )
endif()

