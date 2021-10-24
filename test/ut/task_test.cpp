//
// Created by netcan on 2021/10/11.
//
#include <catch2/catch_test_macros.hpp>
#include <asyncio/event_loop.h>
#include <asyncio/task.h>
#include <functional>

using namespace ASYNCIO_NS;

template<size_t N>
Task<> coro_depth_n(std::vector<int>& result) {
    result.push_back(N);
    if constexpr (N > 0) {
        co_await coro_depth_n<N - 1>(result);
        result.push_back(N * 10);
    }
}

SCENARIO("test Task await") {
    std::vector<int> result;
    EventLoop& loop = get_event_loop();
    GIVEN("simple await") {
        loop.run_until_complete(coro_depth_n<0>(result));
        std::vector<int> expected { 0 };
        REQUIRE(result == expected);
    }

    GIVEN("nest await") {
        loop.run_until_complete(coro_depth_n<1>(result));
        std::vector<int> expected { 1, 0, 10 };
        REQUIRE(result == expected);
    }

    GIVEN("3 depth await") {
        loop.run_until_complete(coro_depth_n<2>(result));
        std::vector<int> expected { 2, 1, 0, 10, 20 };
        REQUIRE(result == expected);
    }

    GIVEN("4 depth await") {
        loop.run_until_complete(coro_depth_n<3>(result));
        std::vector<int> expected { 3, 2, 1, 0, 10, 20, 30 };
        REQUIRE(result == expected);
    }

    GIVEN("5 depth await") {
        loop.run_until_complete(coro_depth_n<4>(result));
        std::vector<int> expected { 4, 3, 2, 1, 0, 10, 20, 30, 40 };
        REQUIRE(result == expected);
    }
}


Task<int64_t> square(int64_t x) {
    co_return x * x;
}

SCENARIO("test Task await result value") {
    EventLoop& loop = get_event_loop();
    GIVEN("square_sum 3, 4") {
        auto square_sum = [&](int x, int y) -> Task<int> {
            auto x2 = co_await square(x);
            auto y2 = co_await square(y);
            co_return x2 + y2;
        };
        REQUIRE(loop.run_until_complete(square_sum(3, 4)) == 25);
    }

    GIVEN("fibonacci") {
        std::function<auto(size_t) -> Task<size_t>> fibo =
            [&](size_t n) -> Task<size_t> {
                if (n <= 1) co_return n;
                co_return co_await fibo(n - 1) +
                          co_await fibo(n - 2);
            };
        REQUIRE(loop.run_until_complete(fibo(0)) == 0);
        REQUIRE(loop.run_until_complete(fibo(1)) == 1);
        REQUIRE(loop.run_until_complete(fibo(2)) == 1);
        REQUIRE(loop.run_until_complete(fibo(12)) == 144);
    }
}

SCENARIO("test Task for loop") {
    EventLoop& loop = get_event_loop();

    auto sequense = [](int64_t n) -> Task<int64_t> {
        int64_t result = 1;
        int64_t sign = -1;
        for (size_t i = 2; i <= n; ++i) {
            result += (co_await square(i)) * sign;
            sign *= -1;
        }
        co_return result;
    };

    REQUIRE(loop.run_until_complete(sequense(1)) == 1);
    REQUIRE(loop.run_until_complete(sequense(10)) == -55);
    REQUIRE(loop.run_until_complete(sequense(100)) == -5050);
    REQUIRE(loop.run_until_complete(sequense(100000)) == -5000050000);
}
