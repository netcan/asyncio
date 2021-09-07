//
// Created by netcan on 2021/09/07.
//

#ifndef ASYNCIO_RUNNER_H
#define ASYNCIO_RUNNER_H
#include <asyncio/asyncio_ns.h>

ASYNCIO_NS_BEGIN
template<typename CORO>
void run(CORO&& main) {
    while (! main.handle_.done()) {
        main.handle_.resume();
    }
}

ASYNCIO_NS_END

#endif // ASYNCIO_RUNNER_H
