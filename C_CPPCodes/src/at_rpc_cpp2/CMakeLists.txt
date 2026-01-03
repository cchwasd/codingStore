cmake_minimum_required(VERSION 3.16)
project(at_rpc_demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 添加 AddressSanitizer 和 Debug 信息 --- dump core file when crash
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# --- 使用 FetchContent 自动下载依赖 ---
# include(FetchContent)

# nlohmann/json
#FetchContent_Declare(
#  json
#  GIT_REPOSITORY https://github.com/nlohmann/json.git
#  GIT_TAG v3.11.2
#)
#FetchContent_MakeAvailable(json)

# spdlog
#FetchContent_Declare(
#  spdlog
#  GIT_REPOSITORY https://github.com/gabime/spdlog.git
#  GIT_TAG v1.12.0
#)
#FetchContent_MakeAvailable(spdlog)

# 添加第三方库的头文件路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- 查找 pthread 库 (用于 std::thread) ---
find_package(Threads REQUIRED)



# --- 定义可执行文件 ---
add_executable(at_rpc_demo
    src/at_rpc_demo.cpp
    src/at_protocol.cpp
    # src/rpc_server.cpp
    # src/rpc_client.cpp
    src/network_utils.cpp
    include/crc32.hpp
)

# --- 链接库 ---
# 添加了 Threads::Threads
target_link_libraries(at_rpc_demo PRIVATE
    # nlohmann_json::nlohmann_json
    # spdlog::spdlog
    Threads::Threads
)

# Windows 额外依赖项
if (WIN32)
    target_link_libraries(at_rpc_demo PRIVATE ws2_32 wsock32)
endif()
# Linux 额外依赖项
if (UNIX AND NOT APPLE)
    # target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif()
# --- 包含头文件目录 ---
target_include_directories(at_rpc_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)
