//
// Created by netcan on 2021/11/24.
//

#ifndef ASYNCIO_RESULT_H
#define ASYNCIO_RESULT_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/exception.h>
#include <variant>
ASYNCIO_NS_BEGIN

template<typename T>
struct Result {
    constexpr bool has_value() {
        return std::get_if<T>(&result_) != nullptr;
    }
    template<typename R>
    constexpr void set_value(R&& value) {
        result_.template emplace<T>(std::forward<R>(value));
    }

    constexpr T& value() & {
        if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }
        if (auto res = std::get_if<T>(&result_)) {
            return *res;
        }
        throw NoResultError{};
    }
    constexpr T&& value() && {
        if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }
        if (auto res = std::get_if<T>(&result_)) {
            return std::move(*res);
        }
        throw NoResultError{};
    }

private:
    std::variant<std::monostate, T, std::exception_ptr> result_;
};

ASYNCIO_NS_END
#endif // ASYNCIO_RESULT_H
