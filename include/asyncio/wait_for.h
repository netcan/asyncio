//
// Created by netcan on 2021/11/21.
//

#ifndef ASYNCIO_WAIT_FOR_H
#define ASYNCIO_WAIT_FOR_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/concept/future.h>
#include <asyncio/concept/awaitable.h>
#include <asyncio/event_loop.h>
#include <asyncio/exception.h>
#include <asyncio/void_value.h>
#include <chrono>
ASYNCIO_NS_BEGIN
namespace detail {
template<typename R, typename Duration>
struct WaitForAwaiter {
    constexpr bool await_ready() noexcept { return false; }
    constexpr R await_resume() const {
        if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }

        if constexpr (! std::is_void_v<R>) {
            if (auto res = std::get_if<R>(&result_)) { return *res; }
            throw std::runtime_error("result is unset");
        }
    }

    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> caller) noexcept {
        continuation_ = &caller.promise();
        // set continuation_ to PENDING, don't schedule anymore, until it resume continuation_
        continuation_->set_state(PromiseState::PENDING);
    }

    template<concepts::Awaitable Fut>
    WaitForAwaiter(Fut&& fut, Duration timeout)
            : wait_for_task_(wait_for_task(no_wait_at_initial_suspend, std::forward<Fut>(fut)))
            , timeout_handle_(*this, timeout, fut.get_resumable())
            { }

private:
    template<concepts::Awaitable Fut>
    Task<> wait_for_task(NoWaitAtInitialSuspend, Fut&& fut) {
        try {
            if constexpr (std::is_void_v<R>) { co_await std::forward<Fut>(fut); }
            else { result_ = std::move(co_await std::forward<Fut>(fut)); }
        } catch(...) {
            result_ = std::current_exception();
        }
        EventLoop& loop{get_event_loop()};
        loop.cancel_handle(timeout_handle_);
        loop.call_soon(*continuation_);
    }

private:
    std::variant<std::monostate, GetTypeIfVoid_t<R>, std::exception_ptr> result_;
    Task<> wait_for_task_;
    Handle* continuation_{};

private:
    struct TimeoutHandle: Handle {
        TimeoutHandle(WaitForAwaiter& awaiter, Duration timeout, Handle& handle)
        : awaiter_(awaiter), current_task_(handle) {
            EventLoop& loop{get_event_loop()};
            loop.call_later(timeout, *this);
        }
        void run() override { // timeout!
            EventLoop& loop{get_event_loop()};
            loop.cancel_handle(current_task_);
            awaiter_.result_ = std::make_exception_ptr(TimeoutError{});
            loop.call_soon(*awaiter_.continuation_);
        }

        WaitForAwaiter& awaiter_;
        Handle& current_task_;
    } timeout_handle_;
};

template<concepts::Awaitable Fut, typename Duration>
WaitForAwaiter(Fut, Duration) -> WaitForAwaiter<AwaitResult<Fut>, Duration>;

template<concepts::Awaitable Fut, typename Duration>
struct WaitForAwaiterRegistry {
    WaitForAwaiterRegistry(Fut&& fut, Duration duration)
    : fut_(std::forward<Fut>(fut)), duration_(duration) { }

    auto operator co_await () && {
        return WaitForAwaiter{std::move(fut_), duration_};
    }
private:
    Fut fut_; // lift Future's lifetime
    Duration duration_;
};

template<concepts::Awaitable Fut, typename Duration>
WaitForAwaiterRegistry(Fut&& fut, Duration duration)
-> WaitForAwaiterRegistry<Fut, Duration>;

template<concepts::Awaitable Fut, typename Rep, typename Period>
auto wait_for(NoWaitAtInitialSuspend, Fut&& fut, std::chrono::duration<Rep, Period> timeout)
-> Task<AwaitResult<Fut>> {
    co_return co_await WaitForAwaiterRegistry { std::forward<Fut>(fut), timeout };
}
}

template<concepts::Awaitable Fut, typename Rep, typename Period>
[[nodiscard("discard wait_for doesn't make sense")]]
auto wait_for(Fut&& fut, std::chrono::duration<Rep, Period> timeout) {
    return detail::wait_for(no_wait_at_initial_suspend, std::forward<Fut>(fut), timeout);
}
ASYNCIO_NS_END
#endif // ASYNCIO_WAIT_FOR_H
