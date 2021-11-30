//
// Created by netcan on 2021/11/30.
//

#ifndef ASYNCIO_STREAM_H
#define ASYNCIO_STREAM_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/noncopyable.h>
#include <asyncio/task.h>
#include <utility>
#include <vector>
#include <unistd.h>

ASYNCIO_NS_BEGIN
struct Stream: NonCopyable {
    using Buffer = std::vector<char>;
    Stream(int fd): fd_(fd) {}
    Stream(Stream&& other): fd_{std::exchange(other.fd_, -1) } {}
    ~Stream() { close(); }

    void close() {
        if (fd_ > 0) { ::close(fd_); }
        fd_ = -1;
    }

    Task<Buffer> read(ssize_t sz = -1) {
        if (sz < 0) { co_return co_await read_until_eof(); }

        Buffer result(sz, 0);
        Event ev { .fd = fd_, .events = EPOLLIN };
        auto& loop = get_event_loop();
        co_await loop.wait_event(ev);
        sz = ::read(fd_, result.data(), result.size());
        if (sz == -1) {
            throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
        }
        result.resize(sz);
        co_return result;
    }

    Task<> write(const Buffer& buf) {
        Event ev { .fd = fd_, .events = EPOLLOUT };
        auto& loop = get_event_loop();
        ssize_t total_write = 0;
        while (total_write < buf.size()) {
            co_await loop.wait_event(ev);
            ssize_t sz = ::write(fd_, buf.data() + total_write, buf.size() - total_write);
            if (sz == -1) {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
            }
            total_write += sz;
        }
    }

private:
    Task<Buffer> read_until_eof() {
        auto& loop = get_event_loop();

        Buffer result(chunk_size, 0);
        Event ev { .fd = fd_, .events = EPOLLIN };
        int current_read = 0;
        int total_read = 0;
        do {
            co_await loop.wait_event(ev);
            current_read = ::read(fd_, result.data() + total_read, chunk_size);
            if (current_read == -1) {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
            }
            if (current_read < chunk_size) { result.resize(total_read + current_read); }
            total_read += current_read;
            result.resize(total_read + chunk_size);
        } while (current_read > 0);
        co_return result;
    }
private:
    int fd_{-1};
    constexpr static size_t chunk_size = 4096;
};
ASYNCIO_NS_END
#endif // ASYNCIO_STREAM_H
