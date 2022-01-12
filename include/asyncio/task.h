//
// Created by netcan on 2021/09/06.
//

#ifndef ASYNCIO_TASK_H
#define ASYNCIO_TASK_H
#include <asyncio/handle.h>
#include <asyncio/event_loop.h>
#include <asyncio/result.h>
#include <asyncio/concept/promise.h>
#include <coroutine>
#include <cassert>
#include <variant>
#include <memory>

ASYNCIO_NS_BEGIN
struct NoWaitAtInitialSuspend {};
inline constexpr NoWaitAtInitialSuspend no_wait_at_initial_suspend;
template<typename R = void>
struct Task: private NonCopyable {
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    void schedule() const {
        return handle_.promise().schedule();
    }
    void cancel() const {
        return handle_.promise().cancel();
    }

    explicit Task(coro_handle h) noexcept: handle_(h) {}
    Task(Task&& t) noexcept
        : handle_(std::exchange(t.handle_, {})) {}

    ~Task() {
        if (handle_) {
            cancel();
            handle_.destroy();
        }
    }

    decltype(auto) get_result() & {
        return handle_.promise().result();
    }

    decltype(auto) get_result() && {
        return std::move(handle_.promise()).result();
    }

    struct AwaiterBase {
        constexpr bool await_ready() {
            if (self_coro_) [[likely]]
            { return self_coro_.done(); }
            return true;
        }
        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> resumer) const noexcept {
            assert(! self_coro_.promise().continuation_);
            resumer.promise().set_state(Handle::SUSPEND);
            self_coro_.promise().continuation_ = &resumer.promise();

            self_coro_.promise().schedule();
        }
        coro_handle self_coro_ {};
    };
    auto operator co_await() const & noexcept {
        struct Awaiter: AwaiterBase {
            decltype(auto) await_resume() const {
                if (! AwaiterBase::self_coro_) [[unlikely]]
                { throw InvalidFuture{}; }
                return AwaiterBase::self_coro_.promise().result();
            }
        };
        return Awaiter {handle_};
    }

    auto operator co_await() const && noexcept {
        struct Awaiter: AwaiterBase {
            decltype(auto) await_resume() const {
                if (! AwaiterBase::self_coro_) [[unlikely]]
                { throw InvalidFuture{}; }
                return std::move(AwaiterBase::self_coro_.promise()).result();
            }
        };
        return Awaiter {handle_};
    }

    struct promise_type: CoroHandle, Result<R> {
        promise_type() = default;

        template<typename... Args> // from free function
        promise_type(NoWaitAtInitialSuspend, Args&&...): wait_at_initial_suspend_{false} { }
        template<typename Obj, typename... Args> // from member function
        promise_type(Obj&&, NoWaitAtInitialSuspend, Args&&...): wait_at_initial_suspend_{false} { }

        auto initial_suspend() noexcept {
            struct InitialSuspendAwaiter {
                constexpr bool await_ready() const noexcept { return !wait_at_initial_suspend_; }
                constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
                constexpr void await_resume() const noexcept {}
                const bool wait_at_initial_suspend_{true};
            };
            return InitialSuspendAwaiter{wait_at_initial_suspend_};
        }
        struct FinalAwaiter {
            constexpr bool await_ready() const noexcept { return false; }
            template<typename Promise>
            constexpr void await_suspend(std::coroutine_handle<Promise> h) const noexcept {
                if (auto cont = h.promise().continuation_) {
                    // SUSPEND -> UNSCHEDULED -> SCHEDULED
                    cont->set_state(Handle::UNSCHEDULED);
                    cont->schedule();
                }
            }
            constexpr void await_resume() const noexcept {}
        };
        auto final_suspend() noexcept {
            return FinalAwaiter {};
        }
        Task get_return_object() noexcept {
            return Task{coro_handle::from_promise(*this)};
        }
        template<concepts::Awaitable A>
        decltype(auto) await_transform(A&& awaiter, // for save source_location info
                                       std::source_location loc = std::source_location::current()) {
            frame_info_ = loc;
            return std::forward<A>(awaiter);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void run() final {
            coro_handle::from_promise(*this).resume();
        }
        const std::source_location& get_frame_info() const final { return frame_info_; }
        void dump_backtrace(size_t depth = 0) const final {
            fmt::print("[{}] {}\n", depth, frame_name());
            if (continuation_) { continuation_->dump_backtrace(depth + 1); }
            else { fmt::print("\n"); }
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

        const bool wait_at_initial_suspend_ {true};
        CoroHandle* continuation_ {};
        std::source_location frame_info_{};
    };

    bool valid() const { return handle_ != nullptr; }
    bool done() const { return handle_.done(); }
private:
    coro_handle handle_;
};

static_assert(concepts::Promise<Task<>::promise_type>);
static_assert(concepts::Future<Task<>>);
ASYNCIO_NS_END
#endif // ASYNCIO_TASK_H
