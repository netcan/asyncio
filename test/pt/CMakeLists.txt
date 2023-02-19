add_executable(sched_test sched_test.cpp)
target_link_libraries(sched_test PRIVATE Catch2WithMain nanobench asyncio)
