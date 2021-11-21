//
// Created by netcan on 2021/11/20.
//

#ifndef ASYNCIO_GATHER_H
#define ASYNCIO_GATHER_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/void_value.h>
#include <asyncio/schedule_task.h>
#include <asyncio/awaitable_traits.h>
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

    template<concepts::Future... Futs>
    GatherAwaiter(Futs&&... futs)
    : GatherAwaiter( std::make_index_sequence<sizeof...(Futs)>{}
                   , std::forward<Futs>(futs)...) { }
private:
    template<concepts::Future... Futs, size_t ...Is>
    GatherAwaiter(std::index_sequence<Is...>, Futs &&... futs)
            : tasks_{ std::make_tuple(collect_result<Is>(std::forward<Futs>(futs))...) } {
        std::apply([]<typename... Ts>(Ts&&... tasks) {
            // use fold expression to guarantee order
            ((void) asyncio::schedule_task(std::forward<Ts>(tasks)), ...);
        }, tasks_);
    }

    template<size_t Idx, typename Fut>
    Task<> collect_result(Fut&& fut) { // TODO: exception handle
        if constexpr (std::is_void_v<AwaitResult<Fut>>) { co_await schedule_task(std::forward<Fut>(fut)); }
        else { std::get<Idx>(result_) = std::move(co_await schedule_task(std::forward<Fut>(fut))); }
        if (++count == sizeof...(Rs) && continuation_) {
            get_event_loop().call_soon(*continuation_);
        }
    }
private:
    [[no_unique_address]] std::tuple<Rs...> result_;
    std::tuple<Task<std::void_t<Rs>>...> tasks_;
    Handle* continuation_{};
    int count{0};
};

template<concepts::Future... Futs> // C++17 deduction guide
GatherAwaiter(Futs&&...) -> GatherAwaiter<GetTypeIfVoid_t<AwaitResult<Futs>>...>;
}

template<concepts::Future... Futs>
auto gather(Futs&&... futs) {
    return detail::GatherAwaiter { std::forward<Futs>(futs)... };
}

ASYNCIO_NS_END
#endif // ASYNCIO_GATHER_H
