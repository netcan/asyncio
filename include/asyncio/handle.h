//
// Created by netcan on 2021/09/08.
//

#ifndef ASYNCIO_HANDLE_H
#define ASYNCIO_HANDLE_H
#include <asyncio/asyncio_ns.h>
#include <cstdint>
#include <source_location>
#include <fmt/core.h>

ASYNCIO_NS_BEGIN
enum class PromiseState: uint8_t {
    UNSCHEDULED,
    PENDING,
};

struct Handle { // type erase for EventLoop
    virtual void run() = 0;
    std::string frame_name() const {
        const auto& frame_info = get_frame_info();
        return fmt::format("{} at {}:{}", frame_info.function_name(),
                           frame_info.file_name(), frame_info.line());
    }
    virtual void dump_backtrace(size_t depth = 0) const {};
    virtual void set_state(PromiseState state) {}
    virtual ~Handle() = default;
private:
    virtual const std::source_location& get_frame_info() const {
        static const std::source_location frame_info = std::source_location::current();
        return frame_info;
    }
};

ASYNCIO_NS_END

#endif // ASYNCIO_HANDLE_H
