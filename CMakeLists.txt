cmake_minimum_required(VERSION 3.10)
project(ringbuf_shm)


include_directories(
    include
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(
    test_shm
    test/test_shm.cpp
)
# set global debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -Wpedantic -Werror")
# OS == linux
if(UNIX AND NOT APPLE)
    target_link_libraries(
        test_shm
        rt
    )
endif()

