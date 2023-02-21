//
// Created by netcan on 2021/09/07.
//
#ifndef ASYNCIO_EVENT_LOOP_H
#define ASYNCIO_EVENT_LOOP_H
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/format.h>
#include <asyncio/handle.h>
#include <asyncio/noncopyable.h>
#include <asyncio/concept/future.h>
#include <asyncio/selector/selector.h>
#include <utility>
#include <unordered_set>
#include <algorithm>
#include <queue>
#include <chrono>
#include <memory>

ASYNCIO_NS_BEGIN
class EventLoop: private NonCopyable {
    using MSDuration = std::chrono::milliseconds;
public:
    EventLoop() {
        auto now = std::chrono::steady_clock::now();
        start_time_ = duration_cast<MSDuration>(now.time_since_epoch());
    }

    MSDuration time() {
        auto now = std::chrono::steady_clock::now();
        return duration_cast<MSDuration>(now.time_since_epoch()) - start_time_;
    }

    template<typename Rep, typename Period>
    void call_later(std::chrono::duration<Rep, Period> delay, Handle& callback) {
        call_at(time() + duration_cast<MSDuration>(delay), callback);
    }

    void cancel_handle(Handle& handle) {
        handle.set_state(Handle::UNSCHEDULED);
        cancelled_.insert(handle.get_handle_id());
    }

    void call_soon(Handle& handle) {
        handle.set_state(Handle::SCHEDULED);
        ready_.push({handle.get_handle_id(), &handle});
    }

    struct WaitEventAwaiter {
        bool await_ready() noexcept {
            bool ready = (event_.handle_info.handle == (const Handle*)&event_.handle_info.handle);
            event_.handle_info.handle = nullptr;
            return ready;
        }
        template<typename Promise>
        constexpr void await_suspend(std::coroutine_handle<Promise> handle) noexcept {
            handle.promise().set_state(Handle::SUSPEND);
            event_.handle_info = {
                .id = handle.promise().get_handle_id(),
                .handle = &handle.promise() //< set callback
            };
            if (! registered_) {
                selector_.register_event(event_);
                registered_ = true;
            }
        }
        void await_resume() noexcept {
            event_.handle_info = { }; //< reset callback
        }

        void destroy() noexcept {
            if (registered_) {
                selector_.remove_event(event_);
                registered_ = false;
            }
        }

        ~WaitEventAwaiter() {
            destroy();
        }

        Selector& selector_;
        Event event_ {};
        bool registered_ { false };
    };

    [[nodiscard]]
    auto wait_event(const Event& event) {
        return WaitEventAwaiter{selector_, event};
    }

    void run_until_complete();

private:
    bool is_stop() {
        return schedule_.empty() && ready_.empty() && selector_.is_stop();
    }

    void cleanup_delayed_call();

    template<typename Rep, typename Period>
    void call_at(std::chrono::duration<Rep, Period> when, Handle& callback) {
        callback.set_state(Handle::SCHEDULED);
        schedule_.emplace_back(duration_cast<MSDuration>(when),
                               HandleInfo{callback.get_handle_id(), &callback});
        std::ranges::push_heap(schedule_, std::ranges::greater{}, &TimerHandle::first);
    }

    void run_once();

private:
    MSDuration start_time_;
    Selector selector_;
    std::queue<HandleInfo> ready_;
    using TimerHandle = std::pair<MSDuration, HandleInfo>;
    std::vector<TimerHandle> schedule_; // min time heap
    std::unordered_set<HandleId> cancelled_;
};

EventLoop& get_event_loop();
ASYNCIO_NS_END

#endif // ASYNCIO_EVENT_LOOP_H
