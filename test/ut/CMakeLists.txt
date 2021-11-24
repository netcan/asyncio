add_executable(asyncio_ut selector_test.cpp task_test.cpp result_test.cpp counted.h)
target_link_libraries(asyncio_ut Catch2WithMain asyncio fmt::fmt)

#include(CTest)
#include(Catch)
#catch_discover_tests(asyncio_ut)