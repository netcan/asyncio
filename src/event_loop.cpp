//
// Created by netcan on 2021/09/07.
//
#include <chrono>
#include <utility>
#include <algorithm>
#include <asyncio/event_loop.h>

namespace ranges = std::ranges;
ASYNCIO_NS_BEGIN
EventLoop& get_event_loop() {
    static EventLoop loop;
    return loop;
}

void EventLoop::run_forever() {
    run_once();
}

void EventLoop::call_at(MSDuration::rep when, std::unique_ptr<resumable> callback) {
    schedule_.emplace_back(std::make_pair(when, std::move(callback)));
    ranges::push_heap(schedule_, ranges::greater{}, &TimerHandle::first);
}

void EventLoop::run_once() {
    MSDuration::rep timeout{0};
    if (! schedule_.empty()) {
        auto&& [when, _] = schedule_[0];
        timeout = when;
    }

    while (! ready_.empty()) {
        auto handle = std::move(ready_.front());
        ready_.pop();
        while (! handle->done())
            handle->resume();
    }
}

Future<void> EventLoop::create_future() {
    co_return;
}

ASYNCIO_NS_END