cmake_minimum_required(VERSION 3.26)
project(Coroutine)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
add_subdirectory(proto)
endif()

#set(protobuf_DIR thirdparty/dist/lib64/cmake/protobuf)
#set(absl_DIR thirdparty/dist/lib64/cmake/absl)
#set(utf8_range_DIR thirdparty/dist/lib64/cmake/utf8_range)
#set(gRPC_DIR thirdparty/dist/lib/cmake/grpc)

#find_package(protobuf CONFIG REQUIRED)
#find_package(gRPC CONFIG REQUIRED)
#find_package(Threads)

add_executable(Coroutine main.cpp
        share_task.hpp)

target_link_libraries(Coroutine pthread)