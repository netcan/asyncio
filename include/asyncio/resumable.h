//
// Created by netcan on 2021/09/08.
//

#ifndef ASYNCIO_RESUMABLE_H
#define ASYNCIO_RESUMABLE_H
#include <asyncio/asyncio_ns.h>

ASYNCIO_NS_BEGIN
struct resumable {
    virtual void resume() = 0;
    virtual bool done() = 0;
    virtual ~resumable() = default;
};
ASYNCIO_NS_END

#endif // ASYNCIO_RESUMABLE_H
