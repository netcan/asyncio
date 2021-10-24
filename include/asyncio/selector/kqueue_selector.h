//
// Created by netcan on 2021/10/24.
//

#ifndef ASYNCIO_KQUEUE_SELECTOR_H
#define ASYNCIO_KQUEUE_SELECTOR_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/selector/event.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <vector>
ASYNCIO_NS_BEGIN
struct KQueueSelector {
    KQueueSelector(): kq_(kqueue()) {
        if (kq_ < 0) {
            perror("kqueue create");
            throw;
        }
        events_.resize(1);
    }

    std::vector<Event> select(size_t timeout /* ms */) {
        errno = 0;
        timespec ts {
            .tv_nsec = static_cast<long>(timeout * 1000000)
        };
        int ndfs = kevent(kq_, nullptr, 0, events_.data(), events_.size(), &ts);
        // TODO: fill the event list
        std::vector<Event> events;
        return events;
    }

    ~KQueueSelector() {
        if (kq_ > 0) {
            close(kq_);
        }
    }
private:
    int kq_;
    std::vector<struct kevent> events_;
};

ASYNCIO_NS_END
#endif //ASYNCIO_KQUEUE_SELECTOR_H
