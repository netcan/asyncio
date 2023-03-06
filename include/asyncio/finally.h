//
// Created by xiaozhuai on 2023/3/6.
//

#ifndef ASYNCIO_FINALLY_H
#define ASYNCIO_FINALLY_H
#include <asyncio/asyncio_ns.h>
#include <functional>
#include <utility>

ASYNCIO_NS_BEGIN

template<class F>
class FinalAction {
public:
    FinalAction(F f) noexcept: f_(std::move(f)), invoke_(true) {}

    FinalAction(FinalAction &&other) noexcept: f_(std::move(other.f_)), invoke_(other.invoke_) {
        other.invoke_ = false;
    }

    FinalAction(const FinalAction &) = delete;

    FinalAction &operator=(const FinalAction &) = delete;

    ~FinalAction() noexcept {
        if (invoke_) f_();
    }

private:
    F f_;
    bool invoke_;
};

template<class F>
inline FinalAction<F> _finally(const F &f) noexcept {
    return FinalAction<F>(f);
}

template<class F>
inline FinalAction<F> _finally(F &&f) noexcept {
    return FinalAction<F>(std::forward<F>(f));
}

#define concat1(a, b)       a ## b
#define concat2(a, b)       concat1(a, b)
#define _finally_object     concat2(_finally_object_, __COUNTER__)
#define finally             ASYNCIO_NS::FinalAction _finally_object = [&]()
#define finally2(func)      ASYNCIO_NS::FinalAction _finally_object = ASYNCIO_NS::_finally(func)

ASYNCIO_NS_END

#endif //ASYNCIO_FINALLY_H
