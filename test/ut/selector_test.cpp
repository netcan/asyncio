//
// Created by netcan on 2021/10/10.
//
#include <catch2/catch_test_macros.hpp>
#include <asyncio/event_loop.h>
#include <asyncio/selector/selector.h>

using namespace ASYNCIO_NS;
using namespace std::chrono;

SCENARIO("test selector wait") {
    EventLoop loop;
    Selector selector;
    auto before_wait = loop.time();
    selector.select(300);
    auto after_wait = loop.time();
    REQUIRE(after_wait - before_wait >= 300ms);
}
