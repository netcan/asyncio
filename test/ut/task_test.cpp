//
// Created by netcan on 2021/10/11.
//
#include <catch2/catch_test_macros.hpp>
#include <asyncio/event_loop.h>
#include <asyncio/task.h>

using namespace ASYNCIO_NS;

struct Dummy {};
Task<Dummy> coro1(std::vector<int>& result) {
    result.push_back(1);
    co_return Dummy {};
}

Task<Dummy> coro2(std::vector<int>& result) {
    result.push_back(2);
    co_await coro1(result);
    result.push_back(20);
    co_return Dummy {};
}

Task<Dummy> coro3(std::vector<int>& result) {
    result.push_back(3);
    co_await coro2(result);
    result.push_back(30);
    co_return Dummy {};
}

Task<Dummy> coro4(std::vector<int>& result) {
    result.push_back(4);
    co_await coro3(result);
    result.push_back(40);
    co_return Dummy {};
}

Task<Dummy> coro5(std::vector<int>& result) {
    result.push_back(5);
    co_await coro4(result);
    result.push_back(50);
    co_return Dummy {};
}

SCENARIO("test Task await") {
    std::vector<int> result;
    EventLoop& loop = get_event_loop();
    GIVEN("simple await") {
        loop.run_until_complete(coro1(result));
        std::vector<int> expected { 1 };
        REQUIRE(result == expected);
    }

    GIVEN("nest await") {
        loop.run_until_complete(coro2(result));
        std::vector<int> expected { 2, 1, 20 };
        REQUIRE(result == expected);
    }

    GIVEN("3 depth await") {
        loop.run_until_complete(coro3(result));
        std::vector<int> expected { 3, 2, 1, 20, 30 };
        REQUIRE(result == expected);
    }

    GIVEN("4 depth await") {
        loop.run_until_complete(coro4(result));
        std::vector<int> expected { 4, 3, 2, 1, 20, 30, 40 };
        REQUIRE(result == expected);
    }

    GIVEN("5 depth await") {
        loop.run_until_complete(coro5(result));
        std::vector<int> expected { 5, 4, 3, 2, 1, 20, 30, 40, 50 };
        REQUIRE(result == expected);
    }

}