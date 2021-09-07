//
// Created by netcan on 2021/09/07.
//

#ifndef ASYNCIO_EVENT_LOOP_H
#define ASYNCIO_EVENT_LOOP_H
#include <asyncio/asyncio_ns.h>

ASYNCIO_NS_BEGIN
struct EventLoop {

};

EventLoop& get_event_loop();
ASYNCIO_NS_END

#endif // ASYNCIO_EVENT_LOOP_H