//
// Created by netcan on 2021/11/20.
//

#ifndef ASYNCIO_GATHER_H
#define ASYNCIO_GATHER_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/void_value.h>
#include <asyncio/awaitable_traits.h>
#include <tuple>
ASYNCIO_NS_BEGIN
namespace detail {
template<typename T>
struct GetResultT: std::type_identity<T> {};
template<>
struct GetResultT<void>: std::type_identity<VoidValue> {};

template<typename T>
using GetResultT_t = typename GetResultT<T>::type;

template<typename... Rs>
struct GatherAwaiter {
    constexpr bool await_ready() noexcept { return false; }
    constexpr auto await_resume() const noexcept { return result_; }
    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> continuation) noexcept {
        continuation_ = &continuation.promise();
    }

    template<typename... Futs, size_t ...Is>
    GatherAwaiter(std::index_sequence<Is...>, Futs &&... futs)
    : tasks_{ std::make_tuple(collect_result<Is>(std::forward<Futs>(futs))...) }
    {
        std::apply([]<typename... Ts>(Ts&&... tasks) {
            // use fold expression to guarantee order
            ((void)asyncio::create_task(std::forward<Ts>(tasks)), ...);
        }, tasks_);
    }

private:
    template<size_t Idx, typename Fut>
    Task<> collect_result(Fut&& fut) {
        if constexpr (std::is_void_v<AwaitResult<Fut>>) {
            co_await create_task(std::forward<Fut>(fut));
        } else {
            std::get<Idx>(result_) = std::move(co_await create_task(std::forward<Fut>(fut)));
        }
        if (++count == sizeof...(Rs) && continuation_) {
            get_event_loop().call_soon(*continuation_);
        }
    }
private:
    Handle* continuation_{};
    std::tuple<Rs...> result_;
    std::tuple<Task<std::void_t<Rs>>...> tasks_;
    int count{0};
};

}

template<typename... Futs>
auto gather(Futs&&... futs) {
    return detail::GatherAwaiter<detail::GetResultT_t<AwaitResult<Futs>>...>
    { std::make_index_sequence<sizeof...(Futs)>{}, std::forward<Futs>(futs)... };
}

ASYNCIO_NS_END
#endif // ASYNCIO_GATHER_H
