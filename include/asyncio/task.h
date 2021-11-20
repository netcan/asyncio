//
// Created by netcan on 2021/09/06.
//

#ifndef ASYNCIO_TASK_H
#define ASYNCIO_TASK_H
#include <asyncio/handle.h>
#include <asyncio/event_loop.h>
#include <coroutine>
#include <cassert>
#include <memory>

ASYNCIO_NS_BEGIN

///////////////////////////////////////////////////////////////////////////////
template<typename R = void>
struct Task: private NonCopyable {
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    Handle& get_resumable() {
        return handle_.promise();
    }

    Task() = default;
    Task(Task&& t) noexcept
        : handle_(std::exchange(t.handle_, {})) {}

    ~Task() {
        if (handle_) {
            if (handle_.done()) { handle_.destroy(); }
            else { handle_.promise().state_ = PromiseState::CANCELED; }
        }
    }

    R get_result() {
        return handle_.promise().result();
    }

    struct Awaiter {
        constexpr bool await_ready() { return self_coro_.done(); }
        R await_resume() const noexcept {
            return self_coro_.promise().result();
        }
        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> resumer) const noexcept {
            assert(! self_coro_.promise().continuation_);
            self_coro_.promise().continuation_ = &resumer.promise();

            if (self_coro_.promise().state_ != PromiseState::PENDING) {
                EventLoop& loop_{get_event_loop()};
                loop_.call_soon(self_coro_.promise());
            }
        }
        coro_handle self_coro_ {};
    };
    auto operator co_await() const noexcept {
        return Awaiter {handle_};
    }

    struct promise_result {
        template<class U>
        void return_value(U &&result) noexcept {
            result_ = std::forward<U>(result);
        }
        R& result() noexcept { return result_; }
    private:
        R result_;
    };

    struct promise_void_result {
        constexpr void return_void() noexcept {}
        constexpr void result() noexcept {}
    };

    using PromiseResult = std::conditional_t<std::is_void_v<R>
            , promise_void_result
            , promise_result>;

    struct promise_type: PromiseResult, Handle {
        auto initial_suspend() noexcept { return std::suspend_always{}; }
        struct FinalAwaiter {
            constexpr bool await_ready() const noexcept { return false; }
            template<typename Promise>
            constexpr void await_suspend(std::coroutine_handle<Promise> h) const noexcept {
                if (h.promise().continuation_) {
                    EventLoop& loop_{get_event_loop()};
                    loop_.call_soon(*h.promise().continuation_);
                }
            }
            constexpr void await_resume() const noexcept {}
        };
        auto final_suspend() noexcept {
            return FinalAwaiter {};
        }
        void unhandled_exception() { std::terminate(); }
        Task get_return_object() noexcept {
            return Task{coro_handle::from_promise(*this)};
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void run() override {
            auto handle = coro_handle::from_promise(*this);
            if (state_ == PromiseState::PENDING) {
                // set to unschedule in advance, because 'resume' may change state
                handle.promise().state_ = PromiseState::UNSCHEDULED;
                handle.resume();
            } else if (state_ == PromiseState::CANCELED) {
                handle.destroy();
            }
        }
        PromiseState& state() override { return state_; }
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // to auto delete by final awaiter
        PromiseState state_ {PromiseState::UNSCHEDULED};
        Handle* continuation_ {};
    };

    explicit Task(coro_handle h) noexcept: handle_(h) {}
    coro_handle handle_;
};

///////////////////////////////////////////////////////////////////////////////
namespace detail {
struct SleepAwaiter {
    constexpr bool await_ready() noexcept { return false; }
    constexpr void await_resume() const noexcept {}
    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> caller) const noexcept {
        auto& loop = get_event_loop();
        loop.call_later(delay_ * 1000,
                        caller.promise());
    }
    double delay_;
};
}

[[nodiscard]]
auto sleep(double delay /* second */) {
    return detail::SleepAwaiter {delay};
}

template<typename Fut>
[[nodiscard("discard(detached) a task will not schedule to run")]]
decltype(auto) create_task(Fut&& fut) {
    auto& loop = get_event_loop();
    loop.call_soon(fut.get_resumable());
    return std::forward<Fut>(fut);
}

ASYNCIO_NS_END
#endif // ASYNCIO_TASK_H