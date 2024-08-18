include_directories(../include)

add_executable(awaiter awiterall.cpp
        ../include/when_all.h
        ../include/fmap.h)