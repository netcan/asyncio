//
// Created by netcan on 2021/09/07.
//

#include <asyncio/event_loop.h>

ASYNCIO_NS_BEGIN
EventLoop& get_event_loop() {
    static EventLoop loop;
    return loop;
}

void EventLoop::run_forever() {
    run_once();
}

void EventLoop::run_once() {
    while (! ready_.empty()) {
        auto handle = std::move(ready_.front());
        ready_.pop();
        while (! handle->done())
            handle->resume();
    }

}

ASYNCIO_NS_END