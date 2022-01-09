//
// Created by netcan on 2021/11/21.
//

#ifndef ASYNCIO_SCHEDULE_TASK_H
#define ASYNCIO_SCHEDULE_TASK_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/concept/future.h>
ASYNCIO_NS_BEGIN

template<concepts::Scheduable Sch>
[[nodiscard("discard(detached) a task will not schedule to run")]]
decltype(auto) schedule_task(Sch&& fut) {
    std::forward<Sch>(fut).schedule();
    return std::forward<Sch>(fut);
}

ASYNCIO_NS_END
#endif // ASYNCIO_SCHEDULE_TASK_H
