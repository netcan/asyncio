//
// Created by netcan on 2021/11/30.
//

#ifndef ASYNCIO_STREAM_H
#define ASYNCIO_STREAM_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/event_loop.h>
#include <asyncio/selector/event.h>
#include <asyncio/noncopyable.h>
#include <asyncio/task.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <utility>
#include <vector>

#if defined(__APPLE__) || defined(__CYGWIN__) || defined(__NetBSD__)
    #include <sys/ioctl.h>
    #include <fcntl.h>
    #ifndef SOCK_NONBLOCK /* __APPLE__: Protocol not supported */
        #define SOCK_NONBLOCK 0
    #endif
#endif

ASYNCIO_NS_BEGIN
namespace socket {
    // Redesign python method `socket.setblocking(bool)`:
    // https://github.com/python/cpython/blob/928752ce4c23f47d3175dd47ecacf08d86a99c9d/Modules/socketmodule.c#L683
    // https://stackoverflow.com/a/1549344/14070318
    bool set_blocking(int fd, bool blocking) {
        if (fd < 0)
            return false;
    #if SOCK_NONBLOCK != 0
        return true;
    #elif defined(_WIN32)
        unsigned long block = !blocking;
        return !ioctlsocket(fd, FIONBIO, &block);
    #elif __has_include(<sys/ioctl.h>) && defined(FIONBIO)
        unsigned int block = !blocking;
        return !ioctl(fd, FIONBIO, &block);
    #else
        int delay_flag, new_delay_flag;
        delay_flag = fcntl(fd, F_GETFL, 0);
        if (delay_flag == -1)
            return false;
        new_delay_flag = blocking ? (delay_flag & ~O_NONBLOCK) : (delay_flag | O_NONBLOCK);
        if (new_delay_flag != delay_flag)
            return !fcntl(fd, F_SETFL, new_delay_flag);
        else
            return false;
    #endif
    }
}

struct Stream: NonCopyable {
    using Buffer = std::vector<char>;
    Stream(int fd): read_fd_(fd), write_fd_(dup(fd)) {
        if (read_fd_ >= 0) {
            socklen_t addrlen = sizeof(sock_info_);
            getsockname(read_fd_, reinterpret_cast<sockaddr*>(&sock_info_), &addrlen);
        }
    }
    Stream(int fd, const sockaddr_storage& sockinfo): read_fd_(fd), write_fd_(dup(fd)), sock_info_(sockinfo) { }
    Stream(Stream&& other): read_fd_{std::exchange(other.read_fd_, -1) },
                            write_fd_{std::exchange(other.write_fd_, -1) },
                            read_ev_{ std::exchange(other.read_ev_, {}) },
                            write_ev_{ std::exchange(other.write_ev_, {}) },
                            read_awaiter_{ std::move(other.read_awaiter_) },
                            write_awaiter_{ std::move(other.write_awaiter_) },
                            sock_info_{ other.sock_info_ } { }
    ~Stream() { close(); }

    void close() {
        read_awaiter_.destroy();
        write_awaiter_.destroy();
        if (read_fd_ > 0) { ::close(read_fd_); }
        if (write_fd_ > 0) { ::close(write_fd_); }
        read_fd_ = -1;
        write_fd_ = -1;
    }

    Task<Buffer> read(ssize_t sz = -1) {
        if (sz < 0) { co_return co_await read_until_eof(); }

        Buffer result(sz, 0);
        co_await read_awaiter_;
        sz = ::read(read_fd_, result.data(), result.size());
        if (sz == -1) {
            throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
        }
        result.resize(sz);
        co_return result;
    }

    Task<> write(const Buffer& buf) {
        auto& loop = get_event_loop();
        ssize_t total_write = 0;
        while (total_write < buf.size()) {
            // FIXME: how to handle write event?
            // co_await write_awaiter_;
            ssize_t sz = ::write(write_fd_, buf.data() + total_write, buf.size() - total_write);
            if (sz == -1) {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
            }
            total_write += sz;
        }
        co_return;
    }
    const sockaddr_storage& get_sock_info() const {
        return sock_info_;
    }

private:
    Task<Buffer> read_until_eof() {
        auto& loop = get_event_loop();

        Buffer result(chunk_size, 0);
        int current_read = 0;
        int total_read = 0;
        do {
            co_await read_awaiter_;
            current_read = ::read(read_fd_, result.data() + total_read, chunk_size);
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
    int read_fd_{-1};
    int write_fd_{-1};
    Event read_ev_ { .fd = read_fd_, .flags = Event::Flags::EVENT_READ };
    Event write_ev_ { .fd = write_fd_, .flags = Event::Flags::EVENT_WRITE };
    EventLoop::WaitEventAwaiter read_awaiter_ { get_event_loop().wait_event(read_ev_) };
    EventLoop::WaitEventAwaiter write_awaiter_ { get_event_loop().wait_event(write_ev_) };
    sockaddr_storage sock_info_{};
    constexpr static size_t chunk_size = 4096;
};


inline const void *get_in_addr(const sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &reinterpret_cast<const sockaddr_in*>(sa)->sin_addr;
    }
    return &reinterpret_cast<const sockaddr_in6*>(sa)->sin6_addr;
}

uint16_t get_in_port(const sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return ntohs(reinterpret_cast<const sockaddr_in*>(sa)->sin_port);
    }

    return ntohs(reinterpret_cast<const sockaddr_in6*>(sa)->sin6_port);
}

ASYNCIO_NS_END
#endif // ASYNCIO_STREAM_H
