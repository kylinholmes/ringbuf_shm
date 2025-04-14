# CMake will be deprecated in the future in this repo, use xmake instead
cmake_minimum_required(VERSION 3.10)
project(ringbuf_shm)

# set global debug
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -Wpedantic -Werror")

include_directories(
    include
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(
    test_shm
    test/test_shm.cpp
)
# OS == linux
if(UNIX AND NOT APPLE)
    target_link_libraries(
        test_shm
        rt pthread
    )
endif()

