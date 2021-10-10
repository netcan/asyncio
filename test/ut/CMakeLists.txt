find_package(Catch2 REQUIRED)

add_executable(asyncio_ut catch_main.cpp epoll_test.cpp)
target_link_libraries(asyncio_ut Catch2::Catch2 asyncio)

include(CTest)
include(Catch)
catch_discover_tests(asyncio_ut)