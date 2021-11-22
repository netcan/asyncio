//
// Created by netcan on 2021/09/08.
//

#ifndef ASYNCIO_FUTURE_H
#define ASYNCIO_FUTURE_H
#include <asyncio/handle.h>
#include <asyncio/concept/awaitable.h>
#include <concepts>

ASYNCIO_NS_BEGIN
namespace concepts {
template<typename Fut>
concept Future = concepts::Awaitable<Fut> && requires(Fut fut) {
    typename std::remove_reference_t<Fut>::promise_type;
    fut.get_result();
};
};
ASYNCIO_NS_END

#endif // ASYNCIO_FUTURE_H
