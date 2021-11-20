//
// Created by netcan on 2021/11/20.
//

#ifndef ASYNCIO_AWAITABLE_TRAITS_H
#define ASYNCIO_AWAITABLE_TRAITS_H
#include <type_traits>
#include <asyncio/asyncio_ns.h>
ASYNCIO_NS_BEGIN
namespace detail {
template<typename A>
struct GetAwaiter: std::type_identity<A> { };

template<typename A>
requires requires(A a) { a.operator co_await(); }
struct GetAwaiter<A>: std::type_identity<decltype(std::declval<A>().operator co_await())> { };

template<typename A>
requires requires(A a) {
    operator co_await(a);
    requires ! (requires { a.operator co_await(); });
}
struct GetAwaiter<A>: std::type_identity<decltype(operator co_await(std::declval<A>()))> { };

template<typename A>
using GetAwaiter_t = typename GetAwaiter<A>::type;
}

template<typename A>
struct AwaitableTraits {
    using AwaitResult = decltype(std::declval<detail::GetAwaiter_t<A>>().await_resume());
};

template<typename A>
using AwaitResult = typename AwaitableTraits<A>::AwaitResult;

ASYNCIO_NS_END
#endif // ASYNCIO_AWAITABLE_TRAITS_H
