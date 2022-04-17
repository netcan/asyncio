//
// Created by netcan on 2021/11/29.
//
#include <asyncio/open_connection.h>
#include <asyncio/runner.h>
#include <asyncio/wait_for.h>
using asyncio::Task;
using asyncio::Stream;
using namespace std::chrono;

Task<> tcp_echo_client(std::string_view message) {
    auto stream = co_await asyncio::open_connection("127.0.0.1", 8888);

    fmt::print("Send: '{}'\n", message);
    co_await stream.write(Stream::Buffer(message.begin(), message.end() + 1 /* plus '\0' */));

    auto data = co_await asyncio::wait_for(stream.read(100), 300ms);
    fmt::print("Received: '{}'\n", data.data());

    fmt::print("Close the connection\n");
    stream.close();
}

int main(int argc, char** argv) {
    asyncio::run(tcp_echo_client("hello world!"));
    return 0;
}
