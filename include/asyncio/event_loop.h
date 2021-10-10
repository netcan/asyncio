//
// Created by netcan on 2021/09/07.
//
#ifndef ASYNCIO_EVENT_LOOP_H
#define ASYNCIO_EVENT_LOOP_H
#include <asyncio/handle.h>
#include <asyncio/noncopyable.h>
#include <asyncio/concept.h>
#include <asyncio/epoll_selector.h>
#include <utility>
#include <algorithm>
#include <queue>
#include <chrono>
#include <memory>

ASYNCIO_NS_BEGIN
class EventLoop: private NonCopyable {
    using MSDuration = std::chrono::milliseconds;

public:
    EventLoop() {
        using namespace std::chrono;
        auto now = system_clock::now();
        start_time_ = duration_cast<MSDuration>(now.time_since_epoch()).count();
    }

    MSDuration::rep time() {
        using namespace std::chrono;
        auto now = system_clock::now();
        return duration_cast<MSDuration>(now.time_since_epoch()).count() - start_time_;
    }

    bool is_stop() {
        return schedule_.empty() && ready_.empty();
    }

    void call_at(MSDuration::rep when, std::unique_ptr<Handle> callback) {
        schedule_.emplace_back(std::make_pair(when, std::move(callback)));
        std::ranges::push_heap(schedule_, std::ranges::greater{}, &TimerHandle::first);
    }

    template<concepts::Coroutine CORO>
    void run_until_complete(CORO&& future) {
        ready_.template emplace(future.get_resumable());
        run_forever();
    }

    void run_forever();

private:
    void run_once();

private:
    MSDuration::rep start_time_;
    std::queue<std::unique_ptr<Handle>> ready_;
    EpollSelector selector_;
    using TimerHandle = std::pair<MSDuration::rep, std::unique_ptr<Handle>>;
    std::vector<TimerHandle> schedule_; // min time heap
};

EventLoop& get_event_loop();
ASYNCIO_NS_END

#endif // ASYNCIO_EVENT_LOOP_H