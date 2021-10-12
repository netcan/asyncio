//
// Created by netcan on 2021/09/06.
//

#ifndef ASYNCIO_TASK_H
#define ASYNCIO_TASK_H
#include <asyncio/handle.h>
#include <asyncio/event_loop.h>
#include <coroutine>
#include <memory>

ASYNCIO_NS_BEGIN

struct CoroHandle: Handle {
    CoroHandle(std::coroutine_handle<> handle): handle_(handle) {}
    void run() override { return handle_.resume(); }
private:
    std::coroutine_handle<> handle_;
};

///////////////////////////////////////////////////////////////////////////////
template<typename R>
struct Task: private NonCopyable {
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    std::unique_ptr<Handle> get_resumable() {
        return std::make_unique<CoroHandle>(handle_);
    }

    Task(Task&& t) noexcept
        : handle_(std::exchange(t.handle_, {})) {}

    ~Task() {
        if (handle_) { handle_.destroy(); }
    }

    auto operator co_await() && noexcept {
        struct Awaiter {
            constexpr bool await_ready() { return false; }
            R await_resume() const noexcept {
                return self_coro_.promise().result_;
            }
            void await_suspend(std::coroutine_handle<> caller) const noexcept {
                self_coro_.promise().continuation_ = caller;
                EventLoop& loop_{get_event_loop()};
                loop_.call_soon(std::make_unique<CoroHandle>(self_coro_));
            }
            coro_handle self_coro_ {};
        };
        return Awaiter {handle_};
    }

    struct promise_type {
        auto initial_suspend() noexcept { return std::suspend_always{}; }
        struct final_awaiter {
            constexpr bool await_ready() const noexcept { return false; }
            template<typename Promise>
            constexpr void await_suspend(std::coroutine_handle<Promise> h) const noexcept {
                if (auto cont = h.promise().continuation_) {
                    EventLoop& loop_{get_event_loop()};
                    loop_.call_soon(std::make_unique<CoroHandle>(cont));
                }
            }
            constexpr void await_resume() const noexcept {}
        };
        auto final_suspend() noexcept {
            return final_awaiter {};
        }
        void unhandled_exception() { std::terminate(); }
        Task get_return_object() noexcept {
            return Task{coro_handle::from_promise(*this)};
        }

        template<class U>
        void return_value(U &&result) noexcept {
            result_ = std::forward<U>(result);
        }

        R &get_result() { return result_; }

        R result_;
        std::coroutine_handle<> continuation_;
    };

private:
    explicit Task(coro_handle h) noexcept: handle_(h) {}
    coro_handle handle_;
};

///////////////////////////////////////////////////////////////////////////////
auto sleep(double delay /* second */) {
    struct Sleep {
        constexpr bool await_ready() noexcept { return false; }
        constexpr void await_resume() const noexcept {}
        void await_suspend(std::coroutine_handle<> caller) const noexcept {
            auto& loop = get_event_loop();
            loop.call_at(loop.time() + delay_ * 1000,
                         std::make_unique<CoroHandle>(caller));
        }
        double delay_;
    };
    return Sleep {delay};
}
ASYNCIO_NS_END
#endif // ASYNCIO_TASK_H