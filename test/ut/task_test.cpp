//
// Created by netcan on 2021/10/11.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <asyncio/event_loop.h>
#include <asyncio/task.h>
#include <asyncio/gather.h>
#include <asyncio/exception.h>
#include <asyncio/sleep.h>
#include <asyncio/schedule_task.h>
#include <asyncio/wait_for.h>
#include <functional>

using namespace ASYNCIO_NS;
using namespace Catch;

template<typename...> struct dump;
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
            auto tx = square(x);
            auto x2 = co_await tx;
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


SCENARIO("test schedule_task") {
    EventLoop& loop = get_event_loop();

    std::vector<int> result;
    auto f = [&]() -> Task<> {
        result.push_back(0xabab);
        co_return;
    };

    GIVEN("run and detach created task") {
        auto test = [&]() -> Task<> {
            auto handle = asyncio::schedule_task(f());
            co_return;
        };
        loop.run_until_complete(test());
        REQUIRE(result.empty());
    }

    GIVEN("run and await created task") {
        auto test = [&]() -> Task<> {
            auto handle = asyncio::schedule_task(f());
            co_await handle;
        };
        loop.run_until_complete(test());
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 0xabab);
    }
}

auto int_div(int a, int b) -> Task<double> {
    if (b == 0) { throw std::overflow_error("b is 0!"); }
    co_return a / b;
};

SCENARIO("test exception") {
    EventLoop& loop = get_event_loop();
    REQUIRE(loop.run_until_complete(int_div(4, 2)) == Approx(2));
    REQUIRE_THROWS_AS(loop.run_until_complete(int_div(4, 0)), std::overflow_error);
}

SCENARIO("test gather") {
    EventLoop& loop = get_event_loop();
    bool is_called = false;
    auto factorial = [&](std::string_view name, int number) -> Task<int> {
        int r = 1;
        for (int i = 2; i <= number; ++i) {
            fmt::print("Task {}: Compute factorial({}), currently i={}...\n", name, number, i);
            co_await asyncio::sleep(0.1s);
            r *= i;
        }
        fmt::print("Task {}: factorial({}) = {}\n", name, number, r);
        co_return r;
    };
    auto test_void_func = []() -> Task<> {
        fmt::print("this is a void value\n");
        co_return;
    };

    SECTION("test lvalue & rvalue gather") {
        REQUIRE(! is_called);
        loop.run_until_complete([&]() -> Task<> {
            auto fac_lvalue = factorial("A", 2);
            auto fac_xvalue = factorial("B", 3);
            auto&& fac_rvalue = factorial("C", 4);
            {
                auto&& [a, b, c, _void] = co_await asyncio::gather(
                        fac_lvalue,
                        static_cast<Task<int>&&>(fac_xvalue),
                        std::move(fac_rvalue),
                        test_void_func()
                );
                REQUIRE(a == 2);
                REQUIRE(b == 6);
                REQUIRE(c == 24);
            }
            REQUIRE((co_await fac_lvalue) == 2);
            REQUIRE(! fac_xvalue.valid());
            REQUIRE(! fac_rvalue.valid());
            is_called = true;
        }());
        REQUIRE(is_called);
    }

    SECTION("test gather of gather") {
        REQUIRE(!is_called);
        loop.run_until_complete([&]() -> Task<> {
            auto&& [ab, c, _void] = co_await asyncio::gather(
                    gather(factorial("A", 2),
                           factorial("B", 3)),
                    factorial("C", 4),
                    test_void_func()
            );
            auto&& [a, b] = ab;
            REQUIRE(a == 2);
            REQUIRE(b == 6);
            REQUIRE(c == 24);
            is_called = true;
        }());
        REQUIRE(is_called);
    }

    SECTION("test detach gather") {
        REQUIRE(! is_called);
        auto res = asyncio::gather(
            factorial("A", 2),
            factorial("B", 3)
        );
        loop.run_until_complete([&]() -> Task<> {
            auto&& [a, b] = co_await std::move(res);
            REQUIRE(a == 2);
            REQUIRE(b == 6);
            is_called = true;
        }());
        REQUIRE(is_called);
    }

    SECTION("test exception gather") {
        REQUIRE(!is_called);
        REQUIRE_THROWS_AS(loop.run_until_complete([&]() -> Task<std::tuple<double, int>> {
            is_called = true;
            co_return co_await asyncio::gather(
                int_div(4, 0),
                factorial("B", 3)
            );
        }()), std::overflow_error);
        REQUIRE(is_called);
    }
}

SCENARIO("test timeout") {
    EventLoop& loop = get_event_loop();

    bool is_called = false;
    auto wait_duration = [&](auto duration) -> Task<int> {
        co_await sleep(duration);
        fmt::print("wait_duration finished\n");
        is_called = true;
        co_return 0xbabababc;
    };

    auto wait_for_test = [&](auto duration, auto timeout) -> Task<int> {
        co_return co_await wait_for(wait_duration(duration), timeout);
    };

    SECTION("no timeout") {
        REQUIRE(! is_called);
        REQUIRE(loop.run_until_complete(wait_for_test(12ms, 120ms)) == 0xbabababc);
        REQUIRE(is_called);
    }

    SECTION("wait_for with sleep") {
        REQUIRE(! is_called);
        loop.run_until_complete([&]() -> Task<> {
            REQUIRE_NOTHROW(co_await wait_for(sleep(30ms), 50ms));
            REQUIRE_THROWS_AS(co_await wait_for(sleep(50ms), 30ms), TimeoutError);
            is_called = true;
        }());
        REQUIRE(is_called);
    }

    SECTION("wait_for with gather") {
        REQUIRE(! is_called);
        loop.run_until_complete([&]() -> Task<> {
            REQUIRE_NOTHROW(co_await wait_for(gather(sleep(10ms), sleep(20ms), sleep(30ms)), 50ms));
            REQUIRE_THROWS_AS(co_await wait_for(gather(sleep(10ms), sleep(80ms), sleep(30ms)), 50ms),
                              TimeoutError);
            is_called = true;
        }());
        REQUIRE(is_called);
    }

    SECTION("notime out with exception") {
        REQUIRE_THROWS_AS(
            loop.run_until_complete([]() -> Task<> {
                auto v = co_await wait_for(int_div(5, 0), 100ms);
            }()), std::overflow_error);
    }

    SECTION("timeout error") {
        REQUIRE(! is_called);
        REQUIRE_THROWS_AS(loop.run_until_complete(wait_for_test(200ms, 100ms)), TimeoutError);
        REQUIRE(! is_called);
    }
}

SCENARIO("test") {
}