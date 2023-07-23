cmake_minimum_required(VERSION 3.18)
set(PROJECT_VERSION 1.0.0)
set(TARGET_NAME "asyncio")
project(${TARGET_NAME}
        VERSION ${PROJECT_VERSION}
        LANGUAGES CXX)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASYNCIO_INC
        include/asyncio/asyncio_ns.h
        include/asyncio/task.h
        include/asyncio/event_loop.h
        include/asyncio/runner.h
        include/asyncio/handle.h
        include/asyncio/concept/future.h
        include/asyncio/noncopyable.h
        include/asyncio/selector/epoll_selector.h
        include/asyncio/selector/kqueue_selector.h
        include/asyncio/selector/event.h
        include/asyncio/selector/selector.h
        include/asyncio/void_value.h
        include/asyncio/exception.h
        include/asyncio/wait_for.h
        include/asyncio/sleep.h
        include/asyncio/schedule_task.h
        include/asyncio/concept/awaitable.h
        include/asyncio/concept/promise.h
        include/asyncio/gather.h
        include/asyncio/result.h
        include/asyncio/callstack.h
        include/asyncio/open_connection.h
        include/asyncio/stream.h
        include/asyncio/start_server.h
        include/asyncio/finally.h
        )

add_library(asyncio SHARED
        ${ASYNC_INC}
        src/event_loop.cpp)

if (BUILD_TESTING)
    add_subdirectory(third_party/Catch2)
    add_subdirectory(third_party/nanobench)
    add_subdirectory(test)
endif()

include(GNUInstallDirs)

find_package(fmt REQUIRED)

set_target_properties(${PROJECT_NAME}
PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME ${PROJECT_NAME}
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
)

target_compile_options(${PROJECT_NAME} PUBLIC -fPIC)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(asyncio PUBLIC fmt::fmt)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/asyncio
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(EXPORT ${TARGET_NAME}Targets
    FILE ${TARGET_NAME}Config.cmake
    NAMESPACE ${TARGET_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/asyncio)
