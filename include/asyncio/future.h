//
// Created by netcan on 2021/09/09.
//

#ifndef ASYNCIO_FUTURE_H
#define ASYNCIO_FUTURE_H
#include <asyncio/asyncio_ns.h>
#include <memory>
#include <coroutine>

ASYNCIO_NS_BEGIN
template<typename R>
struct Future {
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    struct promise_type {
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }
        void unhandled_exception() { std::terminate(); }
        Future get_return_object() {
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

            Future<U> continuation_;
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

template<>
struct Future<void> {
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    struct promise_type {
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }
        void unhandled_exception() { std::terminate(); }
        Future get_return_object() {
            return {coro_handle::from_promise(*this)};
        }

        void return_void() {};
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

            Future<U> continuation_;
        };

    public:
        template<class Cont>
        auto await_transform(Cont continuation) {
            return awaiter{continuation};
        }
    };
    coro_handle handle_;
};


ASYNCIO_NS_END

#endif // ASYNCIO_FUTURE_H
