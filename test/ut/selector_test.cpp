//
// Created by netcan on 2021/10/10.
//
#include <catch2/catch_test_macros.hpp>
#include <asyncio/event_loop.h>
#include <asyncio/Selector.h>

using namespace ASYNCIO_NS;

SCENARIO("test selector wait") {
    EventLoop loop;
    Selector selector;
    int before_wait = loop.time();
    selector.select(300);
    int after_wait = loop.time();
    REQUIRE(after_wait - before_wait >= 300);
}