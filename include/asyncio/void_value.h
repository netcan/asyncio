//
// Created by netcan on 2021/11/20.
//

#ifndef ASYNCIO_VOID_VALUE_H
#define ASYNCIO_VOID_VALUE_H
#include <asyncio/asyncio_ns.h>
ASYNCIO_NS_BEGIN
struct VoidValue { };

namespace detail {
template<typename T>
struct GetTypeIfVoid: std::type_identity<T> {};
template<>
struct GetTypeIfVoid<void>: std::type_identity<VoidValue> {};
}

template<typename T>
using GetTypeIfVoid_t = typename detail::GetTypeIfVoid<T>::type;

#define ASSIGN_VALUE_IF_VOID(res, expr)                                              \
    if constexpr (std::is_same_v<std::remove_reference_t<decltype(res)>, VoidValue>) \
    { expr; } else { res = std::move(expr); }

ASYNCIO_NS_END
#endif // ASYNCIO_VOID_VALUE_H
