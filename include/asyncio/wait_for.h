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
#include <asyncio/schedule_task.h>
#include <asyncio/result.h>
#include <chrono>
ASYNCIO_NS_BEGIN
namespace detail {
template<typename R, typename Duration>
struct WaitForAwaiter: NonCopyable {
    constexpr bool await_ready() noexcept { return result_.has_value(); }
    constexpr decltype(auto) await_resume() {
        return std::move(result_).result();
    }

    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> continuation) noexcept {
        continuation_ = &continuation.promise();
        // set continuation_ to SUSPEND, don't schedule anymore, until it resume continuation_
        continuation_->set_state(Handle::SUSPEND);
    }

    template<concepts::Awaitable Fut>
    WaitForAwaiter(Fut&& fut, Duration timeout)
            : timeout_handle_(*this, timeout)
            , wait_for_task_ {
                schedule_task(wait_for_task(no_wait_at_initial_suspend,
                             std::forward<Fut>(fut)))
            } { }

private:
    template<concepts::Awaitable Fut>
    Task<> wait_for_task(NoWaitAtInitialSuspend, Fut&& fut) {
        try {
            if constexpr (std::is_void_v<R>) { co_await std::forward<Fut>(fut); }
            else { result_.set_value(co_await std::forward<Fut>(fut)); }
        } catch(...) {
            result_.unhandled_exception();
        }
        EventLoop& loop{get_event_loop()};
        loop.cancel_handle(timeout_handle_);
        if (continuation_) {
            loop.call_soon(*continuation_);
        }
    }

private:
    Result<R> result_;
    CoroHandle* continuation_{};

private:
    struct TimeoutHandle: Handle {
        TimeoutHandle(WaitForAwaiter& awaiter, Duration timeout)
        : awaiter_(awaiter) {
            get_event_loop().call_later(timeout, *this);
        }
        void run() final { // timeout!
            awaiter_.wait_for_task_.cancel();
            awaiter_.result_.set_exception(std::make_exception_ptr(TimeoutError{}));

            get_event_loop().call_soon(*awaiter_.continuation_);
        }

        WaitForAwaiter& awaiter_;
    } timeout_handle_;
    ScheduledTask<Task<>> wait_for_task_;
};

template<concepts::Awaitable Fut, typename Duration>
WaitForAwaiter(Fut&&, Duration) -> WaitForAwaiter<AwaitResult<Fut>, Duration>;

template<concepts::Awaitable Fut, typename Duration>
struct WaitForAwaiterRegistry {
    WaitForAwaiterRegistry(Fut&& fut, Duration duration)
    : fut_(std::forward<Fut>(fut)), duration_(duration) { }

    auto operator co_await () && {
        return WaitForAwaiter{std::forward<Fut>(fut_), duration_};
    }
private:
    Fut fut_; // lift Awaitable's lifetime
    Duration duration_;
};

template<concepts::Awaitable Fut, typename Duration>
WaitForAwaiterRegistry(Fut&&, Duration) -> WaitForAwaiterRegistry<Fut, Duration>;

template<concepts::Awaitable Fut, typename Rep, typename Period>
auto wait_for(NoWaitAtInitialSuspend, Fut&& fut, std::chrono::duration<Rep, Period> timeout)
-> Task<AwaitResult<Fut>> { // lift awaitable type(WaitForAwaiterRegistry) to coroutine
    co_return co_await WaitForAwaiterRegistry { std::forward<Fut>(fut), timeout };
}
}

template<concepts::Awaitable Fut, typename Rep, typename Period>
[[nodiscard("discard wait_for doesn't make sense")]]
Task<AwaitResult<Fut>> wait_for(Fut&& fut, std::chrono::duration<Rep, Period> timeout) {
    return detail::wait_for(no_wait_at_initial_suspend, std::forward<Fut>(fut), timeout);
}
ASYNCIO_NS_END
#endif // ASYNCIO_WAIT_FOR_H
