//
// Created by netcan on 2021/11/29.
//

#ifndef ASYNCIO_OPEN_CONNECTION_H
#define ASYNCIO_OPEN_CONNECTION_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/stream.h>
#include <asyncio/addrinfo_guard.h>
#include <asyncio/selector/event.h>
#include <exception>
#include <asyncio/task.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <system_error>

ASYNCIO_NS_BEGIN
namespace detail {
Task<bool> connect(int fd, const sockaddr *addr, socklen_t len) noexcept {
    int rc = ::connect(fd, addr, len);
    if (rc == 0) { co_return true; }
    if (rc < 0 && errno != EINPROGRESS) {
        throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
    }
    Event ev { .fd = fd, .flags = Event::Flags::EVENT_WRITE };
    auto& loop = get_event_loop();
    co_await loop.wait_event(ev);

    int result{0};
    socklen_t result_len = sizeof(result);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &result_len) < 0) {
        // error, fail somehow, close socket
        co_return false;
    }
    co_return result == 0;
}

}

Task<Stream> open_connection(std::string_view ip, uint16_t port) {
    addrinfo hints { .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };
    addrinfo *server_info {nullptr};
    auto service = std::to_string(port);
    // TODO: getaddrinfo is a blocking api
    if (int rv = getaddrinfo(ip.data(), service.c_str(), &hints, &server_info);
            rv != 0) {
        throw std::system_error(std::make_error_code(std::errc::address_not_available));
    }
    AddrInfoGuard _i(server_info);

    int sockfd = -1;
    for (auto p = server_info; p != nullptr; p = p->ai_next) {
        if ((sockfd = ::socket(p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol)) == -1) {
            continue;
        }
        socket::set_blocking(sockfd, false);
        if (co_await detail::connect(sockfd, p->ai_addr, p->ai_addrlen)) {
            break;
        }
        close(sockfd);
        sockfd = -1;
    }
    if (sockfd == -1) {
        throw std::system_error(std::make_error_code(std::errc::address_not_available));
    }

    co_return Stream {sockfd};
}

ASYNCIO_NS_END

#endif // ASYNCIO_OPEN_CONNECTION_H
