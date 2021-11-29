//
// Created by netcan on 2021/11/29.
//
#include <asyncio/open_connection.h>
#include <asyncio/runner.h>
using asyncio::Task;

Task<void> tcp_echo_client(std::string_view message) {
    auto stream = co_await asyncio::open_connection("127.0.0.1", 8888);
    co_return;
}

int main(int argc, char** argv) {
    asyncio::run(tcp_echo_client("hello world!"));
    return 0;
}
