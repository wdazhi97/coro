find_package(Threads)

add_executable(server main.cpp)

target_include_directories(server PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(server
    PRIVATE
        myproto
)