cmake_minimum_required(VERSION 3.16)
project(grpc_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GRPC_INSTALL_DIR "gRPC install prefix (where bin/include/lib are located)" "")


if(GRPC_INSTALL_DIR)
	# find_package(gRPC CONFIG REQUIRED HINTS "${GRPC_INSTALL_DIR}/lib/cmake" "${GRPC_INSTALL_DIR}/cmake")
    # find_package(Protobuf REQUIRED HINTS "${GRPC_INSTALL_DIR}/lib/cmake")
    list(APPEND CMAKE_PREFIX_PATH ${GRPC_INSTALL_DIR})
    find_package(Protobuf REQUIRED)
    find_package(gRPC CONFIG REQUIRED)
else()
	find_package(gRPC CONFIG REQUIRED)
endif()

message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "gRPC package found: ${gRPC_VERSION}")

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protos)
set(PROTO_FILE ${PROTO_DIR}/account.proto)
set(PROTO_OUT_DIR ${PROTO_DIR}/protos_out) # ${CMAKE_CURRENT_BINARY_DIR}

file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

if(GRPC_INSTALL_DIR)
	set(PROTOC_EXECUTABLE "${GRPC_INSTALL_DIR}/bin/protoc")
	if(WIN32)
		set(GRPC_CPP_PLUGIN_EXECUTABLE "${GRPC_INSTALL_DIR}/bin/grpc_cpp_plugin.exe")
	else()
		set(GRPC_CPP_PLUGIN_EXECUTABLE "${GRPC_INSTALL_DIR}/bin/grpc_cpp_plugin")
	endif()
endif()

find_program(PROTOC_EXECUTABLE NAMES protoc HINTS ${PROTOC_EXECUTABLE})
find_program(GRPC_CPP_PLUGIN_EXECUTABLE NAMES grpc_cpp_plugin.exe grpc_cpp_plugin HINTS ${GRPC_CPP_PLUGIN_EXECUTABLE})

if(NOT PROTOC_EXECUTABLE)
	message(FATAL_ERROR "protoc not found. Set GRPC_INSTALL_DIR or ensure protoc is in PATH")
endif()
if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
	message(FATAL_ERROR "grpc_cpp_plugin not found. Set GRPC_INSTALL_DIR or ensure grpc_cpp_plugin is in PATH")
endif()

set(GENERATED_PROTO_SRC ${PROTO_OUT_DIR}/account.pb.cc)
set(GENERATED_PROTO_HDR ${PROTO_OUT_DIR}/account.pb.h)
set(GENERATED_GRPC_SRC ${PROTO_OUT_DIR}/account.grpc.pb.cc)
set(GENERATED_GRPC_HDR ${PROTO_OUT_DIR}/account.grpc.pb.h)

add_custom_command(
	OUTPUT ${GENERATED_PROTO_SRC} ${GENERATED_PROTO_HDR} ${GENERATED_GRPC_SRC} ${GENERATED_GRPC_HDR}
	COMMAND ${PROTOC_EXECUTABLE} --cpp_out=${PROTO_OUT_DIR} -I ${PROTO_DIR} ${PROTO_FILE}
	COMMAND ${PROTOC_EXECUTABLE} --grpc_out=${PROTO_OUT_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE} -I ${PROTO_DIR} ${PROTO_FILE}
	DEPENDS ${PROTO_FILE}
	COMMENT "Generating C++ proto and gRPC sources from ${PROTO_FILE}"
	VERBATIM
)

add_custom_target(generate_proto DEPENDS ${GENERATED_PROTO_SRC} ${GENERATED_GRPC_SRC})

include_directories(${PROTO_OUT_DIR} ${Protobuf_INCLUDE_DIRS})

set(SERVER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/account_server.cpp ${GENERATED_PROTO_SRC} ${GENERATED_GRPC_SRC})
set(CLIENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/account_client.cpp ${GENERATED_PROTO_SRC} ${GENERATED_GRPC_SRC})

add_executable(account_server ${SERVER_SRC})
add_executable(account_client ${CLIENT_SRC})

add_dependencies(account_server generate_proto)
add_dependencies(account_client generate_proto)

target_include_directories(account_server PRIVATE ${PROTO_OUT_DIR})
target_include_directories(account_client PRIVATE ${PROTO_OUT_DIR})

file(GLOB GRPC_LIBS "${GRPC_INSTALL_DIR}/lib/libutf8_*.a")

# 遍历打印找到的库文件
foreach(LIB_FILE ${GRPC_LIBS})
    message(STATUS "Found gRPC related library: ${LIB_FILE}")
endforeach()

target_link_libraries(account_server PRIVATE gRPC::grpc++ gRPC::grpc protobuf::libprotobuf ${GRPC_LIBS}) # ${GRPC_LIBS}
target_link_libraries(account_client PRIVATE gRPC::grpc++ gRPC::grpc protobuf::libprotobuf ${GRPC_LIBS})

message(STATUS "Configured targets: account_server, account_client")



# Usage notes: to force proto regen use: cmake --build build --target generate_proto
# cmake -S . -B build -G "MinGW Makefiles" -DGRPC_INSTALL_DIR="C:/MPrograms/C_CPP/grpc"
# cmake --build build --config Release


