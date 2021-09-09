//
// Created by netcan on 2021/09/06.
//

#ifndef ASYNCIO_TASK_H
#define ASYNCIO_TASK_H
#include <asyncio/resumable.h>
#include <asyncio/future.h>
#include <coroutine>
#include <memory>

ASYNCIO_NS_BEGIN
template<typename R>
struct Task {
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    std::unique_ptr<resumable> get_resumable() {
        struct coro_handle_resumer: resumable {
            coro_handle_resumer(coro_handle handle): handle_(handle) {}
            void resume() override { return handle_.resume(); }
            bool done() override { return handle_.done(); }
            coro_handle handle_;
        };
        return std::make_unique<coro_handle_resumer>(handle_);
    }

    struct promise_type {
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }
        void unhandled_exception() { std::terminate(); }
        Task get_return_object() {
            return {coro_handle::from_promise(*this)};
        }

        template<class U>
        void return_value(U &&result) {
            result_ = std::forward<U>(result);
        }

        R &get_result() { return result_; }

    private:
        template<typename U>
        struct awaiter {
            bool await_ready() { return false; }

            template<typename P>
            void await_suspend(P) {
                if (!continuation_.handle_.done()) {
                    continuation_.handle_.resume();
                }
            }

            U await_resume() {
                return continuation_.handle_.promise().get_result();
            }

            Task<U> continuation_;
        };

    public:
        template<class Cont>
        auto await_transform(Cont continuation) {
            return awaiter{continuation};
        }

    private:

        R result_;
    };

    coro_handle handle_;
};

Future<int> sleep(double delay);
ASYNCIO_NS_END
#endif // ASYNCIO_TASK_H