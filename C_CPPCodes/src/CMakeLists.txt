cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME hrg_log)
project(${PROJECT_NAME} VERSION 0.1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 设置C标准
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -std=c11")

message(STATUS "this is PROJECT ROOT dir " ${CMAKE_SOURCE_DIR}) # CMAKE_SOURCE_DIR
message(STATUS "this is BINARY dir " ${PROJECT_BINARY_DIR})
message(STATUS "this is SOURCE dir " ${PROJECT_SOURCE_DIR})
message(STATUS "this is include dir " ${PROJECT_SOURCE_DIR}/include)

SET(THIRD_PARTY ${CMAKE_SOURCE_DIR}/third_party)


# option(BUILD_DEBUG "Builds debug!!!" OFF)
# # -DBUILD_DEBUG=ON
# if(BUILD_DEBUG)
#     add_compile_options(-g -Wall -Wextra -std=c11)
# endif(BUILD_DEBUG)

# # C的解决办法 中文乱码问题解决
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fexec-charset=GBK")
# # C++ 的解决办法
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fexec-charset=GBK")

# # 设置执行文件，链接库文件输出路径
# SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)
# SET(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)


# # 添加第三方库的头文件路径
# include_directories(${CMAKE_SOURCE_DIR}/include)
# # 添加第三方库的库文件路径
# link_directories(${CMAKE_SOURCE_DIR}/lib)

# 源代码文件
set(SRC_FILES
    # ${CMAKE_SOURCE_DIR}/src/main.cpp
    # ${PROJECT_SOURCE_DIR}/sqlite_demo.cpp
    # ${PROJECT_SOURCE_DIR}/WSqlite.cpp
    ${PROJECT_SOURCE_DIR}/runlog.cpp
    ${PROJECT_SOURCE_DIR}/hrg_log.cpp
)

# 添加子模块
# add_subdirectory(database)
# add_subdirectory(test_code)



# 生成可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC 
    # ${PROJECT_SOURCE_DIR}/include
    # ${CMAKE_SOURCE_DIR}/include
    # ${THIRD_PARTY}/sqlite3/include
    ${THIRD_PARTY}
)

# 链接库文件路径
target_link_directories(${PROJECT_NAME} PUBLIC
    # ${CMAKE_SOURCE_DIR}/lib
    # ${THIRD_PARTY}/sqlite3/lib
    # ${THIRD_PARTY}/spdlog/lib
)

# 指定目标的依赖库
target_link_libraries(${PROJECT_NAME} PUBLIC 
    # sqlite3 pthread
    # spdlog
)

# 设置 编译测试选项
option(BUILD_TEST "Builds test!!!" OFF)
if(BUILD_TEST)
    add_subdirectory(tests)
endif()

# Windows 特定：复制 sqlite3.dll 到可执行文件目录
if(WIN32)
    # add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #     "${THIRD_PARTY}/sqlite3/lib/sqlite3.dll"
    #     $<TARGET_FILE_DIR:${PROJECT_NAME}>
    #     COMMENT "Copying sqlite3.dll to executable directory"
    #     )
endif()