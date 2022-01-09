//
// Created by netcan on 2021/09/07.
//

#ifndef ASYNCIO_RUNNER_H
#define ASYNCIO_RUNNER_H
#include <asyncio/concept/future.h>
#include <asyncio/event_loop.h>

ASYNCIO_NS_BEGIN
template<concepts::Scheduable Sch>
decltype(auto) run(Sch&& main) {
    std::forward<Sch>(main).schedule();
    get_event_loop().run_until_complete();
    return std::forward<Sch>(main).get_result();
}

ASYNCIO_NS_END

#endif // ASYNCIO_RUNNER_H
