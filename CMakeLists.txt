cmake_minimum_required(VERSION 3.18)
set(PROJECT_VERSION 1.0.0)
set(TARGET_NAME "asyncio")
project(${TARGET_NAME}
        VERSION ${PROJECT_VERSION}
        LANGUAGES CXX)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
message(CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

###############################################################################
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
        include/asyncio/addrinfo_guard.h
        )

include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(asyncio SHARED
        ${ASYNC_INC}
        src/event_loop.cpp)
target_link_libraries(asyncio PUBLIC fmt::fmt)

add_subdirectory(test)
add_subdirectory(third_party)

set(MODELS_CMAKE_FILES
    "cmake/asyncioConfig.cmake"
    "cmake/asyncioConfigVersion.cmake"
)

add_compile_options(-fPIC)

set_target_properties(${PROJECT_NAME}
PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME ${PROJECT_NAME}
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
)

set(MODELS_LIBRARY_INSTALL_DIR "/usr/lib/")
set(MODELS_CMAKE_FILES_INSTALL_DIR  "/usr/lib/cmake/asyncio/")
set(MODELS_HEADER_INSTALL_DIR  "/usr/")

# INSTALL CONFIGURATIONS
include(GNUInstallDirs)
set(CMAKE_INSTALL_PREFIX "/usr/")

foreach ( file ${ASYNCIO_INC} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES ${file} DESTINATION "${MODELS_HEADER_INSTALL_DIR}${dir}")
endforeach()

install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${MODELS_CMAKE_FILES}
    DESTINATION ${MODELS_CMAKE_FILES_INSTALL_DIR})    
