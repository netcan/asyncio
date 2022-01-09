//
// Created by netcan on 2021/11/21.
//

#ifndef ASYNCIO_SLEEP_H
#define ASYNCIO_SLEEP_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/task.h>
#include <asyncio/noncopyable.h>
#include <chrono>
ASYNCIO_NS_BEGIN
namespace detail {
template<typename Duration>
struct SleepAwaiter: private NonCopyable {
    explicit SleepAwaiter(Duration delay): delay_(delay) {}
    constexpr bool await_ready() noexcept { return false; }
    constexpr void await_resume() const noexcept {}
    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> caller) const noexcept {
        get_event_loop().call_later(delay_, caller.promise());
    }

private:
    Duration delay_;
};

template<typename Rep, typename Period>
Task<> sleep(NoWaitAtInitialSuspend, std::chrono::duration<Rep, Period> delay) {
    co_await detail::SleepAwaiter {delay};
}
}

template<typename Rep, typename Period>
[[nodiscard("discard sleep doesn't make sense")]]
Task<> sleep(std::chrono::duration<Rep, Period> delay) {
    return detail::sleep(no_wait_at_initial_suspend, delay);
}

using namespace std::chrono_literals;
ASYNCIO_NS_END
#endif // ASYNCIO_SLEEP_H
