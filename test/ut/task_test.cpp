//
// Created by netcan on 2021/10/11.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <asyncio/runner.h>
#include <asyncio/callstack.h>
#include <asyncio/task.h>
#include <asyncio/gather.h>
#include <asyncio/exception.h>
#include <asyncio/sleep.h>
#include <asyncio/schedule_task.h>
#include <asyncio/wait_for.h>
#include <asyncio/start_server.h>
#include <asyncio/open_connection.h>
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
    GIVEN("simple await") {
        asyncio::run(coro_depth_n<0>(result));
        std::vector<int> expected { 0 };
        REQUIRE(result == expected);
    }

    GIVEN("nest await") {
        asyncio::run(coro_depth_n<1>(result));
        std::vector<int> expected { 1, 0, 10 };
        REQUIRE(result == expected);
    }

    GIVEN("3 depth await") {
        asyncio::run(coro_depth_n<2>(result));
        std::vector<int> expected { 2, 1, 0, 10, 20 };
        REQUIRE(result == expected);
    }

    GIVEN("4 depth await") {
        asyncio::run(coro_depth_n<3>(result));
        std::vector<int> expected { 3, 2, 1, 0, 10, 20, 30 };
        REQUIRE(result == expected);
    }

    GIVEN("5 depth await") {
        asyncio::run(coro_depth_n<4>(result));
        std::vector<int> expected { 4, 3, 2, 1, 0, 10, 20, 30, 40 };
        REQUIRE(result == expected);
    }
}

Task<int64_t> square(int64_t x) {
    co_return x * x;
}

SCENARIO("Task<> test") {
    GIVEN("co_await empty task<>") {
        bool called {false};
        try {
            asyncio::run([&]() -> Task<> {
                auto t = square(5);
                auto tt = std::move(t);
                REQUIRE(! t.valid());
                REQUIRE(tt.valid());
                co_await t;
            }());
        } catch(InvalidFuture& ) {
            called = true;
        }

        REQUIRE(called);
    }
}


SCENARIO("test Task await result value") {
    GIVEN("square_sum 3, 4") {
        auto square_sum = [&](int x, int y) -> Task<int> {
            auto tx = square(x);
            auto x2 = co_await tx;
            auto y2 = co_await square(y);
            co_return x2 + y2;
        };
        REQUIRE(asyncio::run(square_sum(3, 4)) == 25);
    }

    GIVEN("fibonacci") {
        std::function<auto(size_t) -> Task<size_t>> fibo =
            [&](size_t n) -> Task<size_t> {
                if (n <= 1) { co_return n; }
                co_return co_await fibo(n - 1) +
                          co_await fibo(n - 2);
            };
        REQUIRE(asyncio::run(fibo(0)) == 0);
        REQUIRE(asyncio::run(fibo(1)) == 1);
        REQUIRE(asyncio::run(fibo(2)) == 1);
        REQUIRE(asyncio::run(fibo(12)) == 144);
    }
}

SCENARIO("test Task for loop") {
    auto sequense = [](int64_t n) -> Task<int64_t> {
        int64_t result = 1;
        int64_t sign = -1;
        for (size_t i = 2; i <= n; ++i) {
            result += (co_await square(i)) * sign;
            sign *= -1;
        }
        co_return result;
    };

    REQUIRE(asyncio::run(sequense(1)) == 1);
    REQUIRE(asyncio::run(sequense(10)) == -55);
    REQUIRE(asyncio::run(sequense(100)) == -5050);
    REQUIRE(asyncio::run(sequense(100000)) == -5000050000);
}


SCENARIO("test schedule_task") {
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
        asyncio::run(test());
        REQUIRE(result.empty());
    }

    GIVEN("run and await created task") {
        auto test = [&]() -> Task<> {
            auto handle = asyncio::schedule_task(f());
            co_await handle;
        };
        asyncio::run(test());
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 0xabab);
    }
}

auto int_div(int a, int b) -> Task<double> {
    if (b == 0) { throw std::overflow_error("b is 0!"); }
    co_return a / b;
};

SCENARIO("test exception") {
    REQUIRE(asyncio::run(int_div(4, 2)) == Approx(2));
    REQUIRE_THROWS_AS(asyncio::run(int_div(4, 0)), std::overflow_error);
}

SCENARIO("test gather") {
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
        asyncio::run([&]() -> Task<> {
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
            REQUIRE(! fac_xvalue.valid()); // be moved
            REQUIRE(! fac_rvalue.valid()); // be moved
            is_called = true;
        }());
        REQUIRE(is_called);
    }

   SECTION("test gather of gather") {
       REQUIRE(!is_called);
       asyncio::run([&]() -> Task<> {
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
       asyncio::run([&]() -> Task<> {
           auto&& [a, b] = co_await std::move(res);
           REQUIRE(a == 2);
           REQUIRE(b == 6);
           is_called = true;
       }());
       REQUIRE(is_called);
   }

   SECTION("test exception gather") {
       REQUIRE(!is_called);
       REQUIRE_THROWS_AS(asyncio::run([&]() -> Task<std::tuple<double, int>> {
           is_called = true;
           co_return co_await asyncio::gather(
               int_div(4, 0),
               factorial("B", 3)
           );
       }()), std::overflow_error);
       REQUIRE(is_called);
   }
}

SCENARIO("test sleep") {
    size_t call_time = 0;
    auto say_after = [&](auto delay, std::string_view what) -> Task<> {
        co_await asyncio::sleep(delay);
        fmt::print("{}\n", what);
        ++call_time;
    };

    GIVEN("schedule sleep and await") {
        auto async_main = [&]() -> Task<> {
            auto task1 = say_after(100ms, "hello");
            task1.schedule();
            auto task2 = say_after(200ms, "world");
            task2.schedule();

            co_await task1;
            co_await task2;
        };
        auto before_wait = get_event_loop().time();
        asyncio::run(async_main());
        auto after_wait = get_event_loop().time();
        auto diff = after_wait - before_wait;
        REQUIRE(diff >= 200ms);
        REQUIRE(diff < 300ms);
        REQUIRE(call_time == 2);
    }

    GIVEN("schedule sleep and cancel") {
        auto async_main = [&]() -> Task<> {
            auto task1 = say_after(100ms, "hello");
            task1.schedule();
            auto task2 = say_after(200ms, "world");
            task2.schedule();

            co_await task1;
            task2.cancel();
        };
        auto before_wait = get_event_loop().time();
        asyncio::run(async_main());
        auto after_wait = get_event_loop().time();
        auto diff = after_wait - before_wait;
        REQUIRE(diff >= 100ms);
        REQUIRE(diff < 200ms);
        REQUIRE(call_time == 1);
    }

    GIVEN("schedule sleep and cancel, delay exit") {
        auto async_main = [&]() -> Task<> {
            auto task1 = say_after(100ms, "hello");
            task1.schedule();
            auto task2 = say_after(200ms, "world");
            task2.schedule();

            co_await task1;
            task2.cancel();
            // delay 300ms to exit
            co_await asyncio::sleep(200ms);
        };
        auto before_wait = get_event_loop().time();
        asyncio::run(async_main());
        auto after_wait = get_event_loop().time();
        auto diff = after_wait - before_wait;
        REQUIRE(diff >= 300ms);
        REQUIRE(diff < 400ms);
        REQUIRE(call_time == 1);
    }
}

SCENARIO("cancel a infinite loop coroutine") {
    int count = 0;
    asyncio::run([&]() -> Task<>{
        auto inf_loop = [&]() -> Task<> {
            while (true) {
                ++count;
                co_await asyncio::sleep(1ms);
            }
        };
        auto task = schedule_task(inf_loop());
        co_await asyncio::sleep(10ms);
        task.cancel();
    }());
    REQUIRE(count > 0);
    REQUIRE(count < 10);
}

SCENARIO("test timeout") {
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
        REQUIRE(asyncio::run(wait_for_test(12ms, 120ms)) == 0xbabababc);
        REQUIRE(is_called);
    }

    SECTION("wait_for with sleep") {
        REQUIRE(! is_called);
        auto wait_for_rvalue = wait_for(sleep(30ms), 50ms);
        asyncio::run([&]() -> Task<> {
            REQUIRE_NOTHROW(co_await std::move(wait_for_rvalue));
            REQUIRE_THROWS_AS(co_await wait_for(sleep(50ms), 30ms), TimeoutError);
            is_called = true;
        }());
        REQUIRE(is_called);
    }

    SECTION("wait_for with gather") {
        REQUIRE(! is_called);
        asyncio::run([&]() -> Task<> {
            REQUIRE_NOTHROW(co_await wait_for(gather(sleep(10ms), sleep(20ms), sleep(30ms)), 50ms));
            REQUIRE_THROWS_AS(co_await wait_for(gather(sleep(10ms), sleep(80ms), sleep(30ms)), 50ms),
                              TimeoutError);
            is_called = true;
        }());
        REQUIRE(is_called);
    }

    SECTION("notime out with exception") {
        REQUIRE_THROWS_AS(
            asyncio::run([]() -> Task<> {
                auto v = co_await wait_for(int_div(5, 0), 100ms);
            }()), std::overflow_error);
    }

    SECTION("timeout error") {
        REQUIRE(! is_called);
        REQUIRE_THROWS_AS(asyncio::run(wait_for_test(200ms, 100ms)), TimeoutError);
        REQUIRE(! is_called);
    }
}

SCENARIO("echo server & client") {
    bool is_called = false;
    constexpr std::string_view message = "hello world!";

    asyncio::run([&]() -> Task<> {
        auto handle_echo = [&](Stream stream) -> Task<> {
            auto& sockinfo = stream.get_sock_info();
            auto data = co_await stream.read(100);
            REQUIRE(std::string_view{data.data()} == message);
            co_await stream.write(data);
        };

        auto echo_server = [&]() -> Task<> {
            auto server = co_await asyncio::start_server(
                    handle_echo, "127.0.0.1", 8888);
            co_await server.server_once();
        };

        auto echo_client = [&]() -> Task<> {
            auto stream = co_await asyncio::open_connection("127.0.0.1", 8888);

            co_await stream.write(Stream::Buffer(message.begin(), message.end()));

            auto data = co_await stream.read(100);
            REQUIRE(std::string_view{data.data()} == message);
            is_called = true;
        };

        auto srv = schedule_task(echo_server());
        auto cli = schedule_task(echo_client());
        co_await cli;
        co_await srv;
    }());

    REQUIRE(is_called);
}

SCENARIO("test") {
}
