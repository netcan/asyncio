//
// Created by netcan on 2021/09/07.
//
#include <chrono>
#include <asyncio/event_loop.h>

namespace ranges = std::ranges;

ASYNCIO_NS_BEGIN
EventLoop& get_event_loop() {
    static EventLoop loop;
    return loop;
}

void EventLoop::run_forever() {
    while (! is_stop()) {
        run_once();
    }
}

void EventLoop::run_once() {
    MSDuration timeout{0};
    if (ready_.empty() && ! schedule_.empty()) {
        auto&& [when, _] = schedule_[0];
        timeout = std::max(when - time(), MSDuration(0));
    }

    auto event_lists = selector_.select(timeout.count());
    // TODO: handle event_lists

    auto end_time = time();
    while (! schedule_.empty()) {
        ranges::pop_heap(schedule_,std::ranges::greater{}, &TimerHandle::first);
        auto&& [when, handle] = schedule_.back();
        if (when >= end_time) break;
        ready_.emplace(handle);
        schedule_.pop_back();
    }

    for (size_t ntodo = ready_.size(), i = 0; i < ntodo ; ++i ) {
        auto handle = ready_.front();
        ready_.pop();
        if (auto iter = cancelled_.find(handle); iter != cancelled_.end()) {
            cancelled_.erase(iter);
        } else {
            handle->run();
        }
    }
}

ASYNCIO_NS_END