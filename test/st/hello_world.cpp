//
// Created by netcan on 2021/09/06.
//
#include <iostream>
#include <fmt/core.h>
#include <string_view>
#include <string>
#include <asyncio/task.h>

Task<std::string_view> world() {
    co_return "world";
}

Task<std::string_view> hello() {
    co_return "hello";
}

Task<std::string> hello_world() {
    auto h = co_await hello();
    auto w = co_await world();
    co_return fmt::format("{} {}", h, w);
}

int main() {
    {
        auto task = hello();
        fmt::print("task done = {}\ntask result = {}\n",
                   task.handle_.done(), task.handle_.promise().get_result());
    }

    {
        auto task = hello_world();
        while (!task.handle_.done())
            task.handle_.resume();
        fmt::print("task done = {}\ntask result = {}\n",
                   task.handle_.done(), task.handle_.promise().get_result());
    }

    return 0;
}