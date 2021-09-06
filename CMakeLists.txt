cmake_minimum_required(VERSION 3.20)
project(asyncio)
find_package(fmt)

set(ASYNCIO_SRCS include/asyncio/task.h)

include_directories(${CMAKE_SOURCE_DIR}/include)
set(CMAKE_CXX_STANDARD 20)

add_executable(hello_world test/st/hello_world.cpp ${AYNCIO_SRCS})
target_link_libraries(hello_world fmt::fmt)