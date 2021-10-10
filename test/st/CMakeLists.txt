find_package(fmt REQUIRED)

add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world asyncio fmt::fmt)