//
// Created by netcan on 2021/12/13.
//
#ifndef ASYNCIO_PROMISE_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/concept/awaitable.h>
ASYNCIO_NS_BEGIN
namespace concepts {
template<typename Promise>
concept promise = requires (Promise p) {
    p.get_return_object();
    { p.initial_suspend() } -> Awaitable;
    { p.final_suspend() } noexcept -> Awaitable;
    p.unhandled_exception();
    requires (requires(int v) { p.return_value(v); } ||
              requires        { p.return_void();   });
};
}
ASYNCIO_NS_END
#endif
