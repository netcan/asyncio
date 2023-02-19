#include <asyncio/stream.h>
#include <arpa/inet.h>
#include <asyncio/runner.h>
#include <asyncio/start_server.h>
#include <asyncio/task.h>
#include <fmt/core.h>

using asyncio::Stream;
using asyncio::Task;
using asyncio::get_in_port;
using asyncio::get_in_addr;

int add_count = 0;
int rel_count = 0;

Task<> handle_echo(Stream stream) {
    auto sockinfo = stream.get_sock_info();
    char addr[INET6_ADDRSTRLEN] {};
    auto sa = reinterpret_cast<const sockaddr*>(&sockinfo);

    ++add_count;
    // fmt::print("connections: {}/{}\n", rel_count, add_count);
    while (true) {
        try {
            auto data = co_await stream.read(200);
            if (data.empty()) { break; }
            // fmt::print("Received: '{}' from '{}:{}'\n", data.data(),
                    // inet_ntop(sockinfo.ss_family, get_in_addr(sa), addr, sizeof addr),
                    // get_in_port(sa));
            co_await stream.write(data);
        } catch (...) {
            break;
        }
    }
    ++rel_count;
    // fmt::print("connections: {}/{}\n", rel_count, add_count);
    stream.close();
}

Task<> echo_server() {
    auto server = co_await asyncio::start_server(
            handle_echo, "127.0.0.1", 8888);

    fmt::print("Serving on 127.0.0.1:8888\n");

    co_await server.serve_forever();
}

int main() {
    asyncio::run(echo_server());
    return 0;
}
