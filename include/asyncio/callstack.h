//
// Created by netcan on 2021/11/25.
//

#ifndef ASYNCIO_CALLSTACK_H
#define ASYNCIO_CALLSTACK_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/noncopyable.h>
#include <chrono>
ASYNCIO_NS_BEGIN
namespace detail {
struct CallStackAwaiter {
    constexpr bool await_ready() noexcept { return false; }
    constexpr void await_resume() const noexcept {}
    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> caller) const noexcept {
        size_t i = 0;
        for (Handle* handle = &caller.promise();
            handle != nullptr;
            handle = handle->get_continuation(), ++i) {
            fmt::print("[{}] {}\n", i, handle->name());
        }
        return false;
    }
};
}

[[nodiscard]] auto dump_callstack() -> detail::CallStackAwaiter { return {}; }

ASYNCIO_NS_END

#endif // ASYNCIO_CALLSTACK_H
