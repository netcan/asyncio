add_executable(hello_world hello_world.cpp)
add_executable(echo_client echo_client.cpp)
add_executable(echo_server echo_server.cpp)

target_link_libraries(hello_world asyncio)
target_link_libraries(echo_client asyncio)
target_link_libraries(echo_server asyncio)
