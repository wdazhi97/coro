include_directories(../include)

add_executable(awaiter awiterall.cpp
        ../include/cancellation_task.hpp)

add_executable(when_all_multi_level
        when_all_multi_level.cpp)


add_executable(cancel cancel.cpp)
add_executable(when_any_test
        when_any_test.cpp
)

target_link_libraries(awaiter pthread)
target_link_libraries(when_all_multi_level pthread)
target_link_libraries(cancel pthread)
target_link_libraries(when_any_test pthread)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
add_subdirectory(grpctest)
endif ()