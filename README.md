<!--ts-->
* [asyncio](#asyncio)
   * [Build &amp; Run](#build--run)
   * [Hello world](#hello-world)
   * [Dump callstack](#dump-callstack)
   * [TCP Echo](#tcp-echo)
      * [Client](#client)
      * [Server](#server)
      * [Benchmark](#benchmark)
   * [Gather](#gather)
   * [Tested Compiler](#tested-compiler)
   * [TODO](#todo)
   * [Reference](#reference)

<!-- Added by: netcan, at: Fri Dec  3 08:06:48 PM HKT 2021 -->

<!--te-->

# asyncio
Asyncio is a C++20 coroutine library to write concurrent code using the await syntax, and imitate python asyncio library.

## Build & Run
```shell
$ git clone --recursive https://github.com/netcan/asyncio.git
$ cd asyncio
$ mkdir build
$ cd build
$ cmake ..
$ make -j
```

## Hello world
```cpp
Task<> hello_world() {
    fmt::print("hello\n");
    co_await asyncio::sleep(1s);
    fmt::print("world\n");
}

int main() {
    asyncio::run(hello_world());
}
```
output:
```shell
hello
world
```

## Dump callstack
```cpp
Task<int> factorial(int n) {
    if (n <= 1) {
        co_await dump_callstack();
        co_return 1;
    }
    co_return (co_await factorial(n - 1)) * n;
}

int main() {
    fmt::print("run result: {}\n", asyncio::run(factorial(10)));
    return 0;
}
```
output:
```shell
[0] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:17
[1] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20
[2] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20
[3] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20
[4] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20
[5] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20
[6] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20
[7] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20
[8] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20
[9] void factorial(factorial(int)::_Z9factoriali.Frame*) at asyncio/test/st/hello_world.cpp:20

run result: 3628800
```

## TCP Echo
### Client
```cpp
Task<> tcp_echo_client(std::string_view message) {
    auto stream = co_await asyncio::open_connection("127.0.0.1", 8888);

    fmt::print("Send: '{}'\n", message);
    co_await stream.write(Stream::Buffer(message.begin(), message.end()));

    auto data = co_await stream.read(100);
    fmt::print("Received: '{}'\n", data.data());

    fmt::print("Close the connection\n");
    stream.close(); // unneeded, just imitate python
}

int main(int argc, char** argv) {
    asyncio::run(tcp_echo_client("hello world!"));
    return 0;
}
```

output:
```shell
Send: 'hello world!'
Received: 'hello world!'
Close the connection
```

### Server
```cpp
Task<> handle_echo(Stream stream) {
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
    stream.close(); // unneeded, just imitate python
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
```

output:
```shell
Serving on 127.0.0.1:8888
Received: 'Hello World!' from '127.0.0.1:49588'
Send: 'Hello World!'
Close the connection
```

### Benchmark
Using the Apache Benchmarking tool, 10000000 requests that each size is 106 byte, 1000 concurrency, enable keepalive, the QPS/RPS result below:

| framework      |  RPS [#/sec] (mean) |
|----------------|--------------------:|
| python asyncio |            47393.59 |
| this project   |       **164457.63** |
| epoll          |           153147.79 |
| libevent       |           136996.46 |
| libuv          |           159937.73 |

The result may be incredible, but it is possible, the magnitude of IO is milliseconds(1e-3 s), while the magnitude of the coroutine is nanoseconds(1e-9 s).

More detail see: [benchmark.md](docs/benchmark.md)

## Gather
```cpp
auto factorial(std::string_view name, int number) -> Task<int> {
    int r = 1;
    for (int i = 2; i <= number; ++i) {
        fmt::print("Task {}: Compute factorial({}), currently i={}...\n", name, number, i);
        co_await asyncio::sleep(500ms);
        r *= i;
    }
    fmt::print("Task {}: factorial({}) = {}\n", name, number, r);
    co_return r;
};

auto test_void_func() -> Task<> {
    fmt::print("this is a void value\n");
    co_return;
};

int main() {
    asyncio::run([&]() -> Task<> {
        auto&& [a, b, c, _void] = co_await asyncio::gather(
            factorial("A", 2),
            factorial("B", 3),
            factorial("C", 4),
            test_void_func());
        assert(a == 2);
        assert(b == 6);
        assert(c == 24);
    }());
}
```

output:
```shell
Task A: Compute factorial(2), currently i=2...
Task B: Compute factorial(3), currently i=2...
Task C: Compute factorial(4), currently i=2...
this is a void value
Task C: Compute factorial(4), currently i=3...
Task A: factorial(2) = 2
Task B: Compute factorial(3), currently i=3...
Task B: factorial(3) = 6
Task C: Compute factorial(4), currently i=4...
Task C: factorial(4) = 24
```

## Tested Compiler
- Debian Linux gcc-11/12, gcc-11 crash at Release mode

## TODO
- [x] implement result type for code reuse, `variant<monostate, value, exception>`
- [x] implement coroutine backtrace(dump continuation chain)
- [x] implement some io coroutine(socket/read/write/close)
- [ ] using libuv as backend

## Reference
- https://github.com/lewissbaker/cppcoro
- https://docs.python.org/3/library/asyncio.html
