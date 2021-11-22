//
// Created by netcan on 2021/11/20.
//

#ifndef ASYNCIO_GATHER_H
#define ASYNCIO_GATHER_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/void_value.h>
#include <asyncio/concept/awaitable.h>
#include <tuple>
ASYNCIO_NS_BEGIN
namespace detail {

template<typename... Rs>
struct GatherAwaiter {
    constexpr bool await_ready() noexcept { return false; }
    constexpr auto await_resume() const noexcept { return result_; }
    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> continuation) noexcept {
        continuation_ = &continuation.promise();
    }

    template<concepts::Awaitable... Futs>
    GatherAwaiter(Futs&&... futs)
    : GatherAwaiter( std::make_index_sequence<sizeof...(Futs)>{}
                   , std::forward<Futs>(futs)...) {}
private:
    template<concepts::Awaitable... Futs, size_t ...Is>
    GatherAwaiter(std::index_sequence<Is...>, Futs&&... futs)
            : tasks_{ std::make_tuple(collect_result<Is>(non_wait_at_initial_suspend, std::forward<Futs>(futs))...) } {
    }

    template<size_t Idx, typename Fut>
    Task<> collect_result(NoWaitAtInitialSuspend, Fut&& fut) { // TODO: exception handle
        if constexpr (std::is_void_v<AwaitResult<Fut>>) { co_await std::forward<Fut>(fut); }
        else { std::get<Idx>(result_) = std::move(co_await std::forward<Fut>(fut)); }
        if (++count == sizeof...(Rs) && continuation_) {
            get_event_loop().call_soon(*continuation_);
        }
    }
private:
    [[no_unique_address]] std::tuple<GetTypeIfVoid_t<Rs>...> result_;
    std::tuple<Task<std::void_t<Rs>>...> tasks_;
    Handle* continuation_{};
    int count{0};
};

template<concepts::Awaitable... Futs> // C++17 deduction guide
GatherAwaiter(Futs&&...) -> GatherAwaiter<AwaitResult<Futs>...>;
}

template<concepts::Awaitable... Futs>
auto gather(Futs&&... futs) -> detail::GatherAwaiter<AwaitResult<Futs>...> {
    return { std::forward<Futs>(futs)... };
}

ASYNCIO_NS_END
#endif // ASYNCIO_GATHER_H
