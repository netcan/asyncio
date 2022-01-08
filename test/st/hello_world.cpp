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

Task<int> factorial(int n) {
  if (n <= 1) {
    co_await dump_callstack();
    co_return 1;
  }
  co_return (co_await factorial(n - 1)) * n;
}

int main() {
  fmt::print("run result: {}\n", asyncio::run(factorial(5)));
}
