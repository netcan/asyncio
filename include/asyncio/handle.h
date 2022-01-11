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
// for cancelled
using HandleId = uint64_t;

struct Handle { // type erase for EventLoop
    enum State: uint8_t {
        UNSCHEDULED,
        SUSPEND,
        SCHEDULED,
    };

    Handle() noexcept: handle_id_(handle_id_generation_++) {}
    virtual void run() = 0;
    virtual void set_state(State state) {}
    HandleId get_handle_id() { return handle_id_; }
    virtual ~Handle() = default;
private:
    HandleId handle_id_;
    static HandleId handle_id_generation_;
};

// handle maybe destroyed, using the increasing id to track the lifetime of handle.
// don't directly using a raw pointer to track coroutine lifetime,
// because a destroyed coroutine may has the same address as a new ready coroutine has created.
struct HandleInfo {
    HandleId id;
    Handle* handle;
};

struct CoroHandle: Handle {
    std::string frame_name() const {
        const auto& frame_info = get_frame_info();
        return fmt::format("{} at {}:{}", frame_info.function_name(),
                           frame_info.file_name(), frame_info.line());
    }
    virtual void dump_backtrace(size_t depth = 0) const {};
    void set_state(State state) final { state_ = state; }
    void schedule();
    void cancel();
private:
    virtual const std::source_location& get_frame_info() const;
protected:
    State state_ {Handle::UNSCHEDULED};
};

ASYNCIO_NS_END

#endif // ASYNCIO_HANDLE_H
