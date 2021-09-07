//
// Created by netcan on 2021/09/07.
//

#include <asyncio/event_loop.h>

ASYNCIO_NS_BEGIN
EventLoop& get_event_loop() {
    static EventLoop loop;
    return loop;
}
ASYNCIO_NS_END