//
// Created by netcan on 2021/11/24.
//

#ifndef ASYNCIO_RESULT_H
#define ASYNCIO_RESULT_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/exception.h>
#include <variant>
#include <optional>
ASYNCIO_NS_BEGIN

template<typename T>
struct Result {
    constexpr bool has_value() const noexcept {
        return std::get_if<T>(&result_) != nullptr;
    }
    template<typename R>
    constexpr void set_value(R&& value) noexcept {
        result_.template emplace<T>(std::forward<R>(value));
    }

    template<typename R> // for promise_type
    constexpr void return_value(R&& value) noexcept {
        return set_value(std::forward<R>(value));
    }

    constexpr T& result() & {
        if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }
        if (auto res = std::get_if<T>(&result_)) {
            return *res;
        }
        throw NoResultError{};
    }
    constexpr T&& result() && {
        if (auto exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }
        if (auto res = std::get_if<T>(&result_)) {
            return std::move(*res);
        }
        throw NoResultError{};
    }

    void set_exception(std::exception_ptr exception) noexcept { result_ = exception; }
    void unhandled_exception() noexcept { result_ = std::current_exception(); }

private:
    std::variant<std::monostate, T, std::exception_ptr> result_;
};

template<>
struct Result<void> {
    bool has_value() const noexcept {
        return result_ != nullptr;
    }
    constexpr void return_void() noexcept { }
    void result() {
        if (result_) { std::rethrow_exception(result_); }
    }

    void set_exception(std::exception_ptr exception) noexcept { result_ = exception; }
    void unhandled_exception() noexcept { result_ = std::current_exception(); }

private:
    std::exception_ptr result_;
};

ASYNCIO_NS_END
#endif // ASYNCIO_RESULT_H
