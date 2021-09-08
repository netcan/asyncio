//
// Created by netcan on 2021/09/07.
//

#ifndef ASYNCIO_EVENT_LOOP_H
#define ASYNCIO_EVENT_LOOP_H
#include <asyncio/resumable.h>
#include <asyncio/noncopyable.h>
#include <asyncio/concept.h>
#include <queue>
#include <memory>

ASYNCIO_NS_BEGIN
struct EventLoop: private NonCopyable {
    template<concepts::Coroutine CORO>
    void run_until_complete(CORO&& future) {
        ready_.template emplace(future.get_resumable());
        run_forever();
    }

    void run_forever();

private:
    void run_once();

private:
    std::queue<std::unique_ptr<resumable>> ready_;
    std::vector<std::unique_ptr<resumable>> schedule_; // min time heap
};

EventLoop& get_event_loop();
ASYNCIO_NS_END

#endif // ASYNCIO_EVENT_LOOP_H