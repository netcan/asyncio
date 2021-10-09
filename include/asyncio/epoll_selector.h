//
// Created by netcan on 2021/10/09.
//

#ifndef ASYNCIO_EPOLL_SELECTOR_H
#define ASYNCIO_EPOLL_SELECTOR_H
#include <asyncio/asyncio_ns.h>
#include <sys/epoll.h>
#include <fmt/core.h>
ASYNCIO_NS_BEGIN
struct EpollSelector {
    EpollSelector(): epfd_(epoll_create1(0)) {
        if (epfd_ < 0) {
            perror("epoll_create1");
            throw;
        }
    }
    ~EpollSelector() {
        if (epfd_ > 0) {
            close(epfd_);
        }
    }
private:
    int epfd_;
};
ASYNCIO_NS_END
#endif // ASYNCIO_EPOLL_SELECTOR_H
