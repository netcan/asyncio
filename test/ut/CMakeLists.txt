add_executable(asyncio_ut epoll_test.cpp task_test.cpp)
target_link_libraries(asyncio_ut Catch2WithMain asyncio fmt::fmt)

#include(CTest)
#include(Catch)
#catch_discover_tests(asyncio_ut)