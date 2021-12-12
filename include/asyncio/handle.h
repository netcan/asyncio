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

// for cancelled
using HandleId = uint64_t;

struct Handle { // type erase for EventLoop
    Handle() noexcept: handle_id_(handle_id_generation_++) {}
    virtual void run() = 0;
    std::string frame_name() const {
        const auto& frame_info = get_frame_info();
        return fmt::format("{} at {}:{}", frame_info.function_name(),
                           frame_info.file_name(), frame_info.line());
    }
    virtual void dump_backtrace(size_t depth = 0) const {};
    virtual void set_state(PromiseState state) {}
    HandleId get_handle_id() { return handle_id_; }
    virtual ~Handle() = default;

private:
    virtual const std::source_location& get_frame_info() const;

private:
    HandleId handle_id_;
    static HandleId handle_id_generation_;
};

ASYNCIO_NS_END

#endif // ASYNCIO_HANDLE_H
