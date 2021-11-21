//
// Created by netcan on 2021/11/21.
//

#ifndef ASYNCIO_SLEEP_H
#define ASYNCIO_SLEEP_H
#include <asyncio/asyncio_ns.h>
ASYNCIO_NS_BEGIN
namespace detail {
struct SleepAwaiter {
    constexpr bool await_ready() noexcept { return false; }
    constexpr void await_resume() const noexcept {}
    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> caller) const noexcept {
        auto& loop = get_event_loop();
        loop.call_later(delay_ * 1000, caller.promise());
    }
    double delay_;
};
}

[[nodiscard]]
auto sleep(double delay /* second */) {
    return detail::SleepAwaiter {delay};
}
ASYNCIO_NS_END
#endif // ASYNCIO_SLEEP_H
