//
// Created by netcan on 2021/11/20.
//

#ifndef ASYNCIO_GATHER_H
#define ASYNCIO_GATHER_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/void_value.h>
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
    constexpr auto await_resume() const noexcept { return result; }
    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> continuation) noexcept {
        continuation_ = std::make_unique<CoroHandle<Promise>>(continuation);
    }

    template<typename... Futs, size_t ...Is>
    GatherAwaiter(std::index_sequence<Is...>, Futs &&... futs)
    : tasks{ std::make_tuple(asyncio::create_task(collect_result<Is>(std::forward<Futs>(futs)))...) }
    { }

private:
    template<size_t Idx, typename Fut>
    Task<> collect_result(Fut&& fut) {
        std::get<Idx>(result) = std::move(co_await create_task(std::forward<Fut>(fut)));
        if (++count == sizeof...(Rs) && continuation_) {
            get_event_loop().call_soon(std::move(continuation_));
        }
    }
private:
    std::unique_ptr<Handle> continuation_;
    std::tuple<Rs...> result;
    std::tuple<Task<std::void_t<Rs>>...> tasks;
    int count{0};
};

}

template<typename... Futs>
auto gather(Futs&&... futs) {
    return detail::GatherAwaiter<detail::GetResultT_t<decltype(futs.operator co_await().await_resume())>...>
    { std::make_index_sequence<sizeof...(Futs)>{}, std::forward<Futs>(futs)... };
}

ASYNCIO_NS_END
#endif // ASYNCIO_GATHER_H
