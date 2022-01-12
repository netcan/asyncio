//
// Created by netcan on 2021/11/20.
//

#ifndef ASYNCIO_GATHER_H
#define ASYNCIO_GATHER_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/task.h>
#include <asyncio/void_value.h>
#include <asyncio/noncopyable.h>
#include <asyncio/concept/awaitable.h>
#include <stdexcept>
#include <tuple>
#include <variant>
ASYNCIO_NS_BEGIN
namespace detail {
template<typename... Rs>
class GatherAwaiter: NonCopyable {
    using ResultTypes = std::tuple<GetTypeIfVoid_t<Rs>...>;
public:
    constexpr bool await_ready() noexcept { return is_finished(); }
    constexpr auto await_resume() const {
        if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }
        if (auto res = std::get_if<ResultTypes>(&result_)) { return *res; }
        throw std::runtime_error("result is unset");
    }
    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> continuation) noexcept {
        continuation_ = &continuation.promise();
        // set continuation_ to SUSPEND, don't schedule anymore, until it resume continuation_
        continuation_->set_state(Handle::SUSPEND);
    }

    template<concepts::Awaitable... Futs>
    explicit GatherAwaiter(Futs&&... futs)
    : GatherAwaiter( std::make_index_sequence<sizeof...(Futs)>{}
                   , std::forward<Futs>(futs)...) {}
private:
    template<concepts::Awaitable... Futs, size_t ...Is>
    explicit GatherAwaiter(std::index_sequence<Is...>, Futs&&... futs)
            : tasks_{ std::make_tuple(collect_result<Is>(no_wait_at_initial_suspend, std::forward<Futs>(futs))...) }
            { }

    template<size_t Idx, concepts::Awaitable Fut>
    Task<> collect_result(NoWaitAtInitialSuspend, Fut&& fut) {
        try {
            auto& results = std::get<ResultTypes>(result_);
            if constexpr (std::is_void_v<AwaitResult<Fut>>) { co_await std::forward<Fut>(fut); }
            else { std::get<Idx>(results) = std::move(co_await std::forward<Fut>(fut)); }
            ++count_;
        } catch(...) {
            result_ = std::current_exception();
        }
        if (is_finished()) {
            get_event_loop().call_soon(*continuation_);
        }
    }
private:
    bool is_finished() {
        return (count_ == sizeof...(Rs)
                || std::get_if<std::exception_ptr>(&result_) != nullptr);
    }
private:
    std::variant<ResultTypes, std::exception_ptr> result_;
    std::tuple<Task<std::void_t<Rs>>...> tasks_;
    CoroHandle* continuation_{};
    int count_{0};
};

template<concepts::Awaitable... Futs> // C++17 deduction guide
GatherAwaiter(Futs&&...) -> GatherAwaiter<AwaitResult<Futs>...>;

template<concepts::Awaitable... Futs>
struct GatherAwaiterRepositry {
    explicit GatherAwaiterRepositry(Futs&&... futs)
    : futs_(std::forward<Futs>(futs)...) { }

    auto operator co_await() && {
        return std::apply([]<concepts::Awaitable... F>(F&&... f) {
            return GatherAwaiter { std::forward<F>(f)... };
        }, std::move(futs_));
    }

private:
    // futs_ to lift Future's lifetime
    // 1. if Future is rvalue(Fut&&), then move it to tuple(Fut)
    // 2. if Future is xvalue(Fut&&), then move it to tuple(Fut)
    // 3. if Future is lvalue(Fut&), then store as lvalue-ref(Fut&)
    std::tuple<Futs...> futs_;
};

template<concepts::Awaitable... Futs> // need deduction guide to deduce future type
GatherAwaiterRepositry(Futs&&...) -> GatherAwaiterRepositry<Futs...>;

template<concepts::Awaitable... Futs>
auto gather(NoWaitAtInitialSuspend, Futs&&... futs) // need NoWaitAtInitialSuspend to lift futures lifetime early
-> Task<std::tuple<GetTypeIfVoid_t<AwaitResult<Futs>>...>> { // lift awaitable type(GatherAwaiterRepositry) to coroutine
    co_return co_await GatherAwaiterRepositry { std::forward<Futs>(futs)... };
}
}

template<concepts::Awaitable... Futs>
[[nodiscard("discard gather doesn't make sense")]]
auto gather(Futs&&... futs) {
    return detail::gather(no_wait_at_initial_suspend, std::forward<Futs>(futs)...);
}

ASYNCIO_NS_END
#endif // ASYNCIO_GATHER_H
