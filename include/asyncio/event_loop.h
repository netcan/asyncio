//
// Created by netcan on 2021/09/07.
//
#ifndef ASYNCIO_EVENT_LOOP_H
#define ASYNCIO_EVENT_LOOP_H
#include <asyncio/handle.h>
#include <asyncio/noncopyable.h>
#include <asyncio/concept/future.h>
#include <asyncio/selector/selector.h>
#include <utility>
#include <set>
#include <algorithm>
#include <queue>
#include <chrono>
#include <memory>

ASYNCIO_NS_BEGIN
class EventLoop: private NonCopyable {
    using MSDuration = std::chrono::milliseconds;

public:
    EventLoop() {
        auto now = std::chrono::system_clock::now();
        start_time_ = duration_cast<MSDuration>(now.time_since_epoch());
    }

    MSDuration time() {
        auto now = std::chrono::system_clock::now();
        return duration_cast<MSDuration>(now.time_since_epoch()) - start_time_;
    }

    bool is_stop() {
        return schedule_.empty() && ready_.empty() && selector_.is_stop();
    }

    template<typename Rep, typename Period>
    void call_later(std::chrono::duration<Rep, Period> delay, Handle& callback) {
        call_at(time() + duration_cast<MSDuration>(delay), callback);
    }

    template<typename Rep, typename Period>
    void call_at(std::chrono::duration<Rep, Period> when, Handle& callback) {
        callback.set_state(PromiseState::PENDING);
        schedule_.emplace_back(std::make_pair(duration_cast<MSDuration>(when), &callback));
        std::ranges::push_heap(schedule_, std::ranges::greater{}, &TimerHandle::first);
    }

    void cancel_handle(Handle& handle) {
        cancelled_.insert(&handle);
    }

    void call_soon(Handle& callback) {
        callback.set_state(PromiseState::PENDING);
        ready_.emplace(&callback);
    }

    template<concepts::Future Fut>
    decltype(auto) run_until_complete(Fut&& future) {
        call_soon(future.get_resumable());
        run_forever();
        return future.get_result();
    }

    struct WaitEventAwaiter {
        constexpr bool await_ready() const noexcept { return false; }
        template<typename Promise>
        constexpr void await_suspend(std::coroutine_handle<Promise> handle) noexcept {
            auto& promise = handle.promise();
            promise.set_state(PromiseState::PENDING);
            event_.data = static_cast<Handle*>(&promise);
            selector_.register_event(event_);
        }
        void await_resume() noexcept {
            selector_.remove_event(event_);
        }

        Selector& selector_;
        Event event_;
    };

    [[nodiscard]]
    auto wait_event(const Event& event) {
        return WaitEventAwaiter(selector_, event);
    }

    void run_forever();

private:
    void run_once();

private:
    MSDuration start_time_;
    std::queue<Handle*> ready_;
    std::set<Handle*> cancelled_;
    Selector selector_;
    using TimerHandle = std::pair<MSDuration, Handle*>;
    std::vector<TimerHandle> schedule_; // min time heap
};

EventLoop& get_event_loop();
ASYNCIO_NS_END

#endif // ASYNCIO_EVENT_LOOP_H