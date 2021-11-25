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

struct HandleFrameInfo {
    uint32_t line{0};
    std::string_view file_name;
    std::string_view func_name;
    constexpr HandleFrameInfo() = default;
    constexpr HandleFrameInfo(std::source_location loc):
        line(loc.line()),
        file_name(loc.file_name()),
        func_name(loc.function_name()) {}
};

struct Handle {
    virtual void run() = 0;
    std::string name() {
        const auto& frame_info = get_frame_info();
//        return fmt::format("{} at {}:{}", frame_info.func_name,
//                           frame_info.file_name, frame_info.line);
        return fmt::format("{} at {}", frame_info.func_name, frame_info.file_name);
    }
    virtual Handle* get_continuation() { return nullptr; }
    virtual void set_state(PromiseState state) {}
    virtual ~Handle() = default;
private:
    virtual const HandleFrameInfo& get_frame_info() = 0;
};

ASYNCIO_NS_END

#endif // ASYNCIO_HANDLE_H
