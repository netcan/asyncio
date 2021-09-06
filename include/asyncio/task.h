//
// Created by netcan on 2021/09/06.
//

#ifndef ASYNCIO_TASK_H
#define ASYNCIO_TASK_H
#include <coroutine>
#include <concepts>

template<typename R>
struct Task {
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;


    struct promise_type {
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }
        void unhandled_exception() { std::terminate(); }
        Task get_return_object() {
            return { coro_handle::from_promise(*this) };
        }

        template<class U>
        void return_value(U&& result) {
            result_ = std::forward<U>(result);
        }

        R& get_result() { return result_; }

    private:
        template<typename U>
        struct awaiter {
            bool await_ready() { return false; }
            template<typename P>
            void await_suspend(P) {
                if (! continuation_.handle_.done()) {
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

#endif // ASYNCIO_TASK_H
