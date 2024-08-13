cmake_minimum_required(VERSION 3.26)
project(Coroutine)

set(CMAKE_CXX_STANDARD 20)

add_executable(Coroutine main.cpp
        share_task.hpp)
