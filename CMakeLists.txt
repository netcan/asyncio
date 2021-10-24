cmake_minimum_required(VERSION 3.18)
project(asyncio)
set(CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
message(CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

###############################################################################
set(ASYNCIO_INC
        include/asyncio/asyncio_ns.h
        include/asyncio/task.h
        include/asyncio/event_loop.h
        include/asyncio/runner.h
        include/asyncio/handle.h
        include/asyncio/concept.h
        include/asyncio/noncopyable.h
        include/asyncio/selector/epoll_selector.h
        include/asyncio/selector/kqueue_selector.h include/asyncio/selector/event.h include/asyncio/selector.h
        )

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(asyncio STATIC
        ${ASYNC_INC}
        src/event_loop.cpp src/task.cpp)

add_subdirectory(test)
add_subdirectory(third_party)
