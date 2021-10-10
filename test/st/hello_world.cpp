//
// Created by netcan on 2021/09/06.
//
#include <iostream>
#include <fmt/core.h>
#include <string_view>
#include <string>
#include <asyncio/task.h>
#include <asyncio/runner.h>
using namespace ASYNCIO_NS;

#if false
Task<std::string_view> world() {
    co_return "world";
}

Task<std::string_view> hello() {
    co_return "hello";
}

Task<std::string> hello_world() {
    auto h = co_await hello();
    auto w = co_await world();
    fmt::print("{} {}", h, w);
    co_return fmt::format("{} {}", h, w);
}
#endif

Task<int> hello_world() {
    fmt::print("hello here 1\n");
    co_await asyncio::sleep(1);
    fmt::print("hello here 2\n");
    co_return 2;
}

int main() {
    asyncio::run(hello_world());

    return 0;
}