//
// Created by netcan on 2021/09/07.
//
#ifndef ASYNCIO_EVENT_LOOP_H
#define ASYNCIO_EVENT_LOOP_H
#include <asyncio/resumable.h>
#include <asyncio/noncopyable.h>
#include <asyncio/concept.h>
#include <asyncio/future.h>
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

    void call_at(MSDuration::rep when, std::unique_ptr<resumable> callback);

    template<concepts::Coroutine CORO>
    void run_until_complete(CORO&& future) {
        ready_.template emplace(future.get_resumable());
        run_forever();
    }

    Future<void> create_future();

    void run_forever();

private:
    void run_once();

private:
    MSDuration::rep start_time_;

private:
    std::queue<std::unique_ptr<resumable>> ready_;
    using TimerHandle = std::pair<MSDuration::rep, std::unique_ptr<resumable>>;
    std::vector<TimerHandle> schedule_; // min time heap
};

EventLoop& get_event_loop();
ASYNCIO_NS_END

#endif // ASYNCIO_EVENT_LOOP_H