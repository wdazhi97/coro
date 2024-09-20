cmake_minimum_required(VERSION 3.26)
project(Coroutine)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

add_executable(Coroutine main.cpp
        share_task.hpp)

target_link_libraries(Coroutine pthread)