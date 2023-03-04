//
// Created by netcan on 2021/10/24.
//

#ifndef ASYNCIO_EVENT_H
#define ASYNCIO_EVENT_H

#include <asyncio/asyncio_ns.h>
#include <asyncio/handle.h>
#include <cstdint>

#if defined(__APPLE__)
    #include <sys/event.h>
    using Flags_t = int16_t;
#elif defined(__linux__)
    #include <sys/epoll.h>
    using Flags_t = uint32_t;
#else
    #error "Support only Linux & MacOS!"
#endif

ASYNCIO_NS_BEGIN
struct Event {
    enum Flags: Flags_t {
    #if defined(__APPLE__)
        EVENT_READ = EVFILT_READ,
        EVENT_WRITE = EVFILT_WRITE
    #elif defined(__linux__)
        EVENT_READ = EPOLLIN,
        EVENT_WRITE = EPOLLOUT
    #else
        #error "Support only Linux & MacOS!"
    #endif
    };

    int fd;
    Flags flags;
    HandleInfo handle_info;
};
ASYNCIO_NS_END

#endif //ASYNCIO_EVENT_H
