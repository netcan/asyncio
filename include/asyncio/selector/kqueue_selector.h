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
#include <chrono>
#include <ranges>
#include <memory>

using namespace std::chrono;

ASYNCIO_NS_BEGIN
struct KQueueSelector {
    KQueueSelector(): kq_(kqueue()) {
        if (kq_ < 0) {
            perror("kqueue create");
            throw;
        }
    }

    std::vector<Event> select(int timeout /* ms */) {
        errno = 0;
        auto nsec = duration_cast<nanoseconds>(milliseconds{ timeout });
        const auto sec = duration_cast<seconds>(nsec);
        const timespec ts {
            .tv_sec = sec.count(),
            .tv_nsec = (nsec - sec).count(),
        };
        std::vector<struct kevent> events(register_event_count_);
        int ndfs = kevent(kq_, nullptr, 0, events.data(), register_event_count_, timeout < 0 ? nullptr : &ts);
        if (ndfs < 0)
            throw std::system_error { errno, std::system_category(), "kevent::select" };

        if (!ndfs)
            return {}; // return empty std::vector<Event>

        std::vector<Event> result;
        if (ndfs >= 100) [[unlikely]] {
            result.reserve(ndfs);
        }
        for (auto event : events | std::views::take(ndfs)) {
            auto handle_info = reinterpret_cast<HandleInfo*>(event.udata);
            if (handle_info->handle != nullptr && handle_info->handle != (Handle*)&handle_info->handle) {
                result.emplace_back(Event {
                    .handle_info = *handle_info
                });
            } else {
                // mark event ready, but has no response callback
                handle_info->handle = (Handle*)&handle_info->handle;
            }
        }
        return result;
    }

    ~KQueueSelector() {
        if (kq_ > 0) {
            close(kq_);
        }
    }

    bool is_stop() const {
        return register_event_count_ == 1;
    }

    void register_event(const Event& event) {
        struct kevent ev {
            .ident = static_cast<uintptr_t>(event.fd),
            .filter = static_cast<int16_t>(event.flags),
            .flags = EV_ADD | EV_ENABLE,
            .udata = const_cast<HandleInfo*>(&event.handle_info)
        };
        if (!kevent(kq_, &ev, 1, nullptr, 0, nullptr)) {
            ++register_event_count_;
        }
    }

    void remove_event(const Event& event) {
        struct kevent ev {
            .ident = static_cast<uintptr_t>(event.fd),
            .filter = event.flags,
            .flags = EV_DELETE | EV_DISABLE,
            .udata = nullptr
        };
        if (!kevent(kq_, &ev, 1, nullptr, 0, nullptr)) {
            --register_event_count_;
        }
    }

private:
    int kq_;
    int register_event_count_ { 1 };
    /* FIXME: Is there a needed? With zero (0) faster than one (1).
    "When Fa nevents is zero, kevent ();
will return immediately even if there is a Fa timeout specified unlike select(2)."
    https://www.opennet.ru/man.shtml?topic=kevent
    *BUT* asyncio/test/ut/selector_test.cpp not pass if use zero (0).
    */
};

ASYNCIO_NS_END
#endif //ASYNCIO_KQUEUE_SELECTOR_H
