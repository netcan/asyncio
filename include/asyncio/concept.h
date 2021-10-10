//
// Created by netcan on 2021/09/08.
//

#ifndef ASYNCIO_CONCEPT_H
#define ASYNCIO_CONCEPT_H
#include <asyncio/handle.h>
#include <concepts>

ASYNCIO_NS_BEGIN
namespace concepts {
template<typename CORO>
concept Coroutine = requires(CORO coro) {
    { *coro.get_resumable() } -> std::convertible_to<Handle&>;
};

};
ASYNCIO_NS_END

#endif // ASYNCIO_CONCEPT_H
