//
// Created by netcan on 2021/10/10.
//
#include <catch2/catch_test_macros.hpp>
#include <asyncio/event_loop.h>
#include <asyncio/epoll_selector.h>

using namespace ASYNCIO_NS;

SCENARIO("test epoll wait") {
    EventLoop loop;
    EpollSelector selector;
    int before_wait = loop.time();
    selector.select(300);
    int after_wait = loop.time();
    REQUIRE(after_wait - before_wait >= 300);
}