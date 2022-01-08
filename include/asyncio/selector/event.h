//
// Created by netcan on 2021/10/24.
//

#ifndef ASYNCIO_EVENT_H
#define ASYNCIO_EVENT_H

#include <asyncio/asyncio_ns.h>
#include <asyncio/handle.h>
#include <cstdint>
ASYNCIO_NS_BEGIN
struct Event {
    int fd;
    uint32_t events;
    HandleInfo handle_info;
};
ASYNCIO_NS_END

#endif //ASYNCIO_EVENT_H
