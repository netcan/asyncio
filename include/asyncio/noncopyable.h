//
// Created by netcan on 2021/09/08.
//

#ifndef ASYNCIO_NONCOPYABLE_H
#define ASYNCIO_NONCOPYABLE_H
#include <asyncio/asyncio_ns.h>

ASYNCIO_NS_BEGIN
struct NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
ASYNCIO_NS_END

#endif // ASYNCIO_NONCOPYABLE_H
