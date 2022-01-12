//
// Created by netcan on 2021/09/07.
//

#ifndef ASYNCIO_RUNNER_H
#define ASYNCIO_RUNNER_H
#include <asyncio/concept/future.h>
#include <asyncio/event_loop.h>
#include <asyncio/schedule_task.h>

ASYNCIO_NS_BEGIN
template<concepts::Future Fut>
decltype(auto) run(Fut&& main) {
    auto t = schedule_task(std::forward<Fut>(main));
    get_event_loop().run_until_complete();
    if constexpr (std::is_lvalue_reference_v<Fut&&>) {
        return t.get_result();
    } else {
        return std::move(t).get_result();
    }
}

ASYNCIO_NS_END

#endif // ASYNCIO_RUNNER_H
