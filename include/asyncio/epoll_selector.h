//
// Created by netcan on 2021/10/09.
//

#ifndef ASYNCIO_EPOLL_SELECTOR_H
#define ASYNCIO_EPOLL_SELECTOR_H
#include <asyncio/asyncio_ns.h>
#include <sys/epoll.h>
#include <vector>
#include <fmt/core.h>
ASYNCIO_NS_BEGIN
struct EpollSelector {
    struct Event {
        int fd;
        uint32_t events;
    };
    EpollSelector(): epfd_(epoll_create1(0)) {
        if (epfd_ < 0) {
            perror("epoll_create1");
            throw;
        }
        events_.resize(1);
    }
    std::vector<Event> select(size_t timeout /* ms */) {
        errno = 0;
        int ndfs = epoll_wait(epfd_, events_.data(), events_.size(), timeout);
        // TODO: fill the event list
        std::vector<Event> events;
        return events;
    }
    ~EpollSelector() {
        if (epfd_ > 0) {
            close(epfd_);
        }
    }
private:
    int epfd_;
    std::vector<epoll_event> events_;
};
ASYNCIO_NS_END
#endif // ASYNCIO_EPOLL_SELECTOR_H
