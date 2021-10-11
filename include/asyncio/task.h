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
    ~CoroHandle() override {
        if (handle_.done()) { handle_.destroy(); }
    }
private:
    std::coroutine_handle<> handle_;
};

///////////////////////////////////////////////////////////////////////////////
template<typename R>
struct Task {
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    std::unique_ptr<Handle> get_resumable() {
        return std::make_unique<CoroHandle>(handle_);
    }

    auto operator co_await() {
        struct Awaiter {
            constexpr bool await_ready() { return false; }
            R await_resume() const noexcept {
                // TODO: fill Result value
                return R{};
            }
            void await_suspend(std::coroutine_handle<> caller) const noexcept {
                struct CallAndReturn: Handle {
                    CallAndReturn(coro_handle self, std::coroutine_handle<> caller):
                            self_(self), caller_(caller) {}
                    void run() override {
                        self_.resume();
                        loop_.call_soon(std::make_unique<CoroHandle>(caller_));
                    }
                    ~CallAndReturn() override {
                        if (self_.done()) { self_.destroy(); }
                    }
                    coro_handle self_;
                    std::coroutine_handle<> caller_ {};
                    EventLoop& loop_{get_event_loop()};
                };
                loop_.call_soon(std::make_unique<CallAndReturn>(self_, caller));
            }
            coro_handle self_ {};
            EventLoop& loop_{get_event_loop()};
        };
        return Awaiter {handle_};
    }

    struct promise_type {
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
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
        R result_;
    };

    coro_handle handle_;
};

///////////////////////////////////////////////////////////////////////////////
auto sleep(double delay /* second */) {
    struct Sleep {
        constexpr bool await_ready() { return false; }
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