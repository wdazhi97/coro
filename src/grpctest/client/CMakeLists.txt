find_package(Threads)

add_executable(client main.cpp)

target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(client PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(client
    PRIVATE
        myproto
)


add_executable(clientasync client_async.cpp)

target_include_directories(clientasync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(clientasync PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(clientasync
    PRIVATE
        myproto
)