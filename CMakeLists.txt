cmake_minimum_required(VERSION 3.20)
project(asyncio)
set(CMAKE_CXX_STANDARD 20)

find_package(fmt)

###############################################################################

set(ASYNCIO_INC
        include/asyncio/asyncio_ns.h
        include/asyncio/task.h
        include/asyncio/event_loop.h
        include/asyncio/runner.h
        include/asyncio/resumable.h
        )

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(asyncio STATIC
        ${ASYNC_INC}
        src/event_loop.cpp include/asyncio/concept.h include/asyncio/noncopyable.h)

add_executable(hello_world test/st/hello_world.cpp ${AYNCIO_INC})
target_link_libraries(hello_world asyncio fmt::fmt)