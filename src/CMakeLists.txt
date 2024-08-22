include_directories(../include)

add_executable(awaiter awiterall.cpp
        ../include/when_all.h
        ../include/fmap.h
        ../include/cancellation_task.hpp)

add_executable(when_all_multi_level
        when_all_multi_level.cpp)


add_executable(cancel cancel.cpp)
add_executable(when_any_test
        when_any_test.cpp
)
