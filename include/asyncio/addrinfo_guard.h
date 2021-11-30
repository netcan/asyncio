//
// Created by netcan on 2021/11/30.
//

#ifndef ASYNCIO_ADDRINFO_GUARD_H
#define ASYNCIO_ADDRINFO_GUARD_H
#include <asyncio/asyncio_ns.h>
#include <netdb.h>

ASYNCIO_NS_BEGIN
struct AddrInfoGuard {
    AddrInfoGuard(addrinfo* info): info_(info) { }
    ~AddrInfoGuard() { freeaddrinfo(info_); }
private:
    addrinfo* info_{nullptr};
};
ASYNCIO_NS_END

#endif // ASYNCIO_ADDRINFO_GUARD_H
