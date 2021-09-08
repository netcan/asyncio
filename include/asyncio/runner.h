//
// Created by netcan on 2021/09/07.
//

#ifndef ASYNCIO_RUNNER_H
#define ASYNCIO_RUNNER_H
#include <asyncio/concept.h>
#include <asyncio/event_loop.h>

ASYNCIO_NS_BEGIN
template<concepts::Coroutine CORO>
void run(CORO&& main) {
    decltype(auto) loop = get_event_loop();
    return loop.run_until_complete(std::forward<CORO>(main));
}

ASYNCIO_NS_END

#endif // ASYNCIO_RUNNER_H
