//
// Created by netcan on 2021/09/06.
//
#include <iostream>
#include <fmt/core.h>
#include <string_view>
#include <string>
#include <asyncio/task.h>
#include <asyncio/runner.h>
#include <asyncio/sleep.h>
#include <asyncio/schedule_task.h>
#include <asyncio/callstack.h>
using namespace ASYNCIO_NS;

Task<std::string_view> hello() {
    fmt::print("enter {}...\n", __FUNCTION__ );
    co_await asyncio::sleep(1s);
    co_await dump_callstack();
    fmt::print("exit {}...\n", __FUNCTION__ );
    co_return "hello";
}

Task<std::string_view> world() {
    fmt::print("enter {}...\n", __FUNCTION__ );
    co_await asyncio::sleep(500ms);
    fmt::print("exit {}...\n", __FUNCTION__ );
    co_return "world";
}


Task<std::string> hello_world() {
    auto h = asyncio::schedule_task(hello());
    auto w = asyncio::schedule_task(world());
    co_return fmt::format("{} {}\n", co_await h, co_await w);
}

int main() {
    fmt::print("run result: {}\n", asyncio::run(hello_world()));
    return 0;
}