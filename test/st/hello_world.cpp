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

Task<std::string_view> hello() {
    fmt::print("enter {}...\n", __FUNCTION__ );
    co_await asyncio::sleep(3);
    fmt::print("exit {}...\n", __FUNCTION__ );
    co_return "hello";
}

Task<std::string_view> world() {
    fmt::print("enter {}...\n", __FUNCTION__ );
    co_await asyncio::sleep(2);
    fmt::print("exit {}...\n", __FUNCTION__ );
    co_return "world";
}


Task<std::string> hello_world() {
    auto h = asyncio::create_task(hello());
    auto w = asyncio::create_task(world());

    co_return fmt::format("{} {}\n", co_await h, co_await w);
}

int main() {
    fmt::print("run result: {}\n", asyncio::run(hello_world()));

    return 0;
}