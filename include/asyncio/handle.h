//
// Created by netcan on 2021/09/08.
//

#ifndef ASYNCIO_HANDLE_H
#define ASYNCIO_HANDLE_H
#include <asyncio/asyncio_ns.h>
#include <cstdint>

ASYNCIO_NS_BEGIN
enum class PromiseState: uint8_t {
    UNSCHEDULED,
    PENDING,
    CANCELED,
};

struct Handle {
    virtual void run() = 0;
    virtual void set_state(PromiseState) = 0;
    virtual ~Handle() = default;
};
ASYNCIO_NS_END

#endif // ASYNCIO_HANDLE_H
