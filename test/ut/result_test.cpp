//
// Created by netcan on 2021/11/24.
//
#include <catch2/catch_test_macros.hpp>
#include <asyncio/result.h>
#include <asyncio/event_loop.h>
#include <asyncio/task.h>
#include "counted.h"

using namespace ASYNCIO_NS;

SCENARIO("test Counted") {
    using TestCounted = Counted<default_counted_policy>;
    TestCounted::reset_count();
    GIVEN("move counted") {
        {
            TestCounted c1;
            TestCounted c2(std::move(c1));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 0);
    }

    GIVEN("copy counted") {
        {
            TestCounted c1;
            TestCounted c2(c1);
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::copy_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 0);
    }

    GIVEN("copy counted") {
        TestCounted c1;
        {
            TestCounted c2(std::move(c1));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(c1.id_ == -1);
    }
}

SCENARIO("test result T") {
    using TestCounted = Counted<{
        .move_assignable=false,
        .copy_assignable=false
    }>;
    TestCounted::reset_count();
    GIVEN("result set lvalue") {
        Result<TestCounted> res;
        REQUIRE(! res.has_value());
        {
            TestCounted c;
            REQUIRE(TestCounted::construct_counts() == 1);
            REQUIRE(TestCounted::copy_construct_counts == 0);
            res.set_value(c);
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::copy_construct_counts == 1);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(res.has_value());
    }

    GIVEN("result set rvalue") {
        Result<TestCounted> res;
        REQUIRE(! res.has_value());
        {
            TestCounted c;
            REQUIRE(TestCounted::construct_counts() == 1);
            REQUIRE(TestCounted::move_construct_counts == 0);
            res.set_value(std::move(c));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(res.has_value());
    }

    GIVEN("lvalue result") {
        Result<TestCounted> res;
        res.set_value(TestCounted{});
        REQUIRE(res.has_value());
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 0);
        {
            {
                auto&& r = res.result();
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::move_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 1);
            }
            {
                auto r = res.result();
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 2);
            }
        }
        REQUIRE(TestCounted::alive_counts() == 1);
    }

    GIVEN("rvalue result") {
        Result<TestCounted> res;
        res.set_value(TestCounted{});
        REQUIRE(res.has_value());
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        {
            auto r = std::move(res).result();
            REQUIRE(TestCounted::move_construct_counts == 2);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
    }


}


SCENARIO("test pass parameters to the coroutine frame") {
    EventLoop& loop = get_event_loop();
    using TestCounted = Counted<{
        .move_assignable=false,
        .copy_assignable=false
    }>;
    TestCounted::reset_count();

    GIVEN("pass by rvalue") {
        auto coro = [](TestCounted count) -> Task<> {
            REQUIRE(count.alive_counts() == 2);
            co_return;
        };
        loop.run_until_complete(coro(TestCounted{}));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::alive_counts() == 0);
    }

    GIVEN("pass by lvalue") {
        auto coro = [](TestCounted count) -> Task<> {
            REQUIRE(TestCounted::copy_construct_counts == 1);
            REQUIRE(TestCounted::move_construct_counts == 1);
            REQUIRE(count.alive_counts() == 3);
            co_return;
        };
        TestCounted count;
        loop.run_until_complete(coro(count));

        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(count.id_ != -1);
    }

    GIVEN("pass by xvalue") {
        auto coro = [](TestCounted count) -> Task<> {
            REQUIRE(TestCounted::copy_construct_counts == 0);
            REQUIRE(TestCounted::move_construct_counts == 2);
            REQUIRE(count.alive_counts() == 3);
            REQUIRE(count.id_ != -1);
            co_return;
        };
        TestCounted count;
        loop.run_until_complete(coro(std::move(count)));

        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 0);
        REQUIRE(TestCounted::move_construct_counts == 2);
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(count.id_ == -1);
    }

    GIVEN("pass by lvalue ref") {
        auto coro = [](TestCounted& count) -> Task<> {
            REQUIRE(count.alive_counts() == 1);
            co_return;
        };
        TestCounted count;
        loop.run_until_complete(coro(count));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::construct_counts() == 1);
        REQUIRE(TestCounted::alive_counts() == 1);
    }

    GIVEN("pass by rvalue ref") {
        auto coro = [](TestCounted&& count) -> Task<> {
            REQUIRE(count.alive_counts() == 1);
            co_return;
        };
        loop.run_until_complete(coro(TestCounted{}));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::construct_counts() == 1);
        REQUIRE(TestCounted::alive_counts() == 0);
    }
}
