//
// Created by netcan on 2021/09/07.
//

#ifndef ASYNCIO_RUNNER_H
#define ASYNCIO_RUNNER_H
#include <asyncio/concept/future.h>
#include <asyncio/event_loop.h>

ASYNCIO_NS_BEGIN
template<concepts::Future Fut>
decltype(auto) run(Fut&& main) {
    auto& loop = get_event_loop();
    loop.call_soon(main.get_resumable());
    loop.run_until_complete();
    return main.get_result();
}

ASYNCIO_NS_END

#endif // ASYNCIO_RUNNER_H
