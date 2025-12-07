cmake_minimum_required(VERSION 3.16.3)
project(BuildExample VERSION 0.1.0)

# enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 设置为C语言项目
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD_REQUIRED ON)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -std=c11")

# option(BUILD_DEBUG "Builds debug!!!" OFF)
# # -DBUILD_DEBUG=ON
# if(BUILD_DEBUG)
#     add_compile_options(-g -Wall -Wextra -std=c11)
#     message(STATUS "Build Debug Mode")
# endif(BUILD_DEBUG)

# # C的解决办法 中文乱码问题解决
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fexec-charset=GBK")
# # C++ 的解决办法
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fexec-charset=GBK")

# 设置执行文件，链接库文件输出路径
SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)
SET(THIRD_PARTY ${CMAKE_SOURCE_DIR}/third_party)

set(MYSQL_ROOT_DIR "C:/MPrograms/Database/mysql-8.3.0")

# # 添加第三方库的头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${THIRD_PARTY}/sqlite3/include
    ${MYSQL_ROOT_DIR}/include
    ${THIRD_PARTY}
)
# # 添加第三方库的库文件路径
# link_directories(${CMAKE_SOURCE_DIR}/lib)



# 添加子模块
# add_subdirectory(src/socket_use)
# add_subdirectory(src/database)
add_subdirectory(src)

# 将子目录src添加进工程并编译，同时指定二进制目标文件的输出路径为bin目录
# add_subdirectory(src) #  ${LIBRARY_OUTPUT_PATH}

# 指定目标的依赖库
# target_link_libraries(${OUTPUT_NAME} PUBLIC pthread)

# # 设置 编译测试选项
# option(BUILD_TEST "Builds test!!!" OFF)
# if(BUILD_TEST)
#     add_subdirectory(tests)
# endif()

# build cmd:
# cmake -S . -B build  -G "MinGW Makefiles" -DBUILD_DEBUG=ON
# cmake --build build --config Debug --target all -- -j 4