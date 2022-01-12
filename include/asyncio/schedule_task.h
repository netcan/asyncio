//
// Created by netcan on 2021/11/21.
//

#ifndef ASYNCIO_SCHEDULE_TASK_H
#define ASYNCIO_SCHEDULE_TASK_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/noncopyable.h>
#include <asyncio/concept/future.h>
ASYNCIO_NS_BEGIN

template<concepts::Future Task>
struct ScheduledTask: private NonCopyable {
    template<concepts::Future Fut>
    explicit ScheduledTask(Fut&& fut): task_(std::forward<Fut>(fut)) {
        if (task_.valid() && ! task_.done()) {
            task_.handle_.promise().schedule();
        }
    }

    void cancel() { task_.~Task(); }

    decltype(auto) operator co_await() const & noexcept {
        return task_.operator co_await();
    }

    auto operator co_await() const && noexcept {
        return task_.operator co_await();
    }

    decltype(auto) get_result() & {
        return task_.get_result();
    }

    decltype(auto) get_result() && {
        return std::move(task_).get_result();
    }

    bool valid() const { return task_.valid(); }
    bool done() const { return task_.done(); }

private:
    Task task_;
};

template<concepts::Future Fut>
ScheduledTask(Fut&&) -> ScheduledTask<Fut>;

template<concepts::Future Fut>
[[nodiscard("discard(detached) a task will not schedule to run")]]
ScheduledTask<Fut> schedule_task(Fut&& fut) {
    return ScheduledTask { std::forward<Fut>(fut) };
}

ASYNCIO_NS_END
#endif // ASYNCIO_SCHEDULE_TASK_H
