//
// Created by netcan on 2021/11/30.
//

#include <asyncio/runner.h>
#include <asyncio/start_server.h>
#include <arpa/inet.h>

using asyncio::Task;
using asyncio::Stream;
using asyncio::get_in_addr;
using asyncio::get_in_port;

Task<void> handle_echo(Stream stream) {
    auto& sockinfo = stream.get_sock_info();
    auto sa = reinterpret_cast<const sockaddr*>(&sockinfo);
    char addr[INET6_ADDRSTRLEN] {};

    auto data = co_await stream.read(100);
    fmt::print("Received: '{}' from '{}:{}'\n", data.data(),
               inet_ntop(sockinfo.ss_family, get_in_addr(sa), addr, sizeof addr),
               get_in_port(sa));

    fmt::print("Send: '{}'\n", data.data());
    co_await stream.write(data);

    fmt::print("Close the connection\n");
    stream.close();
}

Task<void> amain() {
    auto server = co_await asyncio::start_server(
            handle_echo, "127.0.0.1", 8888);

    fmt::print("Serving on 127.0.0.1:8888\n");

    co_await server.serve_forever();
}

int main() {
    asyncio::run(amain());
    return 0;
}
