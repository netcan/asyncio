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
   * [FAQ](#faq)
   * [Reference](#reference)

<!-- Added by: netcan, at: Tue Dec  7 07:54:16 PM HKT 2021 -->

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

| framework      |  RPS [#/sec] (mean) | Language |   Pattern |
|----------------|--------------------:| --------: |----------:|
| [python asyncio](docs/benchmark.md#python399-asyncio)             | 47393.59       | Python             | coroutine |
| [python asyncio with uvloop](docs/benchmark.md#python399-asyncio) | 100426.97      | Python             | coroutine |
| [this project](docs/benchmark.md#this-project)                    | **164457.63**  | C++20              | coroutine |
| [asio](docs/benchmark.md#asio1180-in-coroutine-mode)              | 159322.66      | C++20              | coroutine |
| [tokio-rs](docs/benchmark.md#tokio-rs-1140)                       | 156852.70      | Rust1.59.0-nightly | coroutine |
| [epoll](docs/benchmark.md#c-epoll-version)                        | 153147.79      | C                  | eventloop |
| [libevent](docs/benchmark.md#c-libevent-21so7)                    | 136996.46      | C                  |  callback |
| [libuv](docs/benchmark.md#c-libuv1420)                            | 159937.73      | C                  |  callback |

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

## FAQ
Source:
- [https://www.reddit.com/r/cpp/comments/r5oz8q/asyncio_imitate_pythons_asyncio_for_c/](https://www.reddit.com/r/cpp/comments/r5oz8q/asyncio_imitate_pythons_asyncio_for_c/)
- [https://www.reddit.com/r/cpp/comments/r7xvd1/c20_coroutine_benchmark_result_using_my_coroutine/](https://www.reddit.com/r/cpp/comments/r7xvd1/c20_coroutine_benchmark_result_using_my_coroutine/)

> **Q**: technically, you can add a handle that doesn't exist in the event_loop queue. Would the cancelled event become a dangler in such a scenario?
> ```cpp
> void cancel_handle(Handle& handle) {
>     cancelled_.insert(&handle);
> }
> ```
> **A**: it maybe memory leak at some scenario but it's safe, the cancelled set stores handle was destroyed, it notices eventloop when handle was readying, just skip it and remove from cancelled set prevent some memory leaks.
>
> **A**: you are right, I find a bug at release mode when a handle is destroyed and inserted into the cancelled set, and then another coroutine is created, it has the same address as the destroyed coroutine handle!!! The loop will remove the new ready coroutine had created.

> **Q**: First off, great work! Do you have any suggestions for understanding when to use coroutines and when to not use them? They're too new to see what kind of performance they bring to the table, and I don't see much in terms of comparisons with other methods yet.
>
> **A**: good question.  for my point, the coroutine is just a syntax-sugar for callback, in other words, any scenario that requires callback interfaces can be replaced by the coroutine, a typical asynchronous programming pattern involves a lot of callbacks, so use coroutine the code is very readable than callback style.
>
> for performance, coroutine is just a resumable function, it supports suspend and resume, I tested coroutine call/suspend/resume, it costs only a few tens of ns, compare to callback style programming, it has negative overhead than callback, more compare detail see: https://www.youtube.com/watch?v=_fu0gx-xseY, the author of coroutine.
>
> **Q**: Thanks for that insight. I am an embedded developer, so most of my callbacks are hardware/interrupt driven. But I suppose I could create pseudo-interrupts with this if I wanted to run simulations on non-realtime hardware? I think I might just need to give them a shot to better understand.
>
> **A**: you can think that pseudo interrupts handle just call a resume() of a coroutine handle after prepare some interrupt's data, then control flow will given to a point that wait(await) relative interrupt data and continue to process. Another scenario I found a good post for help understand that consider an embedded device that monitors data values, such as temperature, and writes these values to a serial port (RS232) complete with a timestamp, which could be time from device boot or a network synchronised clock time, https://blog.feabhas.com/2021/09/c20-coroutines/
>
> **Q**: The experience so far seems that Gor was a bit too optimistic with that statements: in practice it seems to really depend whether the compiler manages to optimize away the coroutine, or not.
>
> **A**: As far as I'm exploring, current compiler doesn't do HALO(Heap Allocation eLision Optimization,http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0981r0.html), so compare to normal function call, it costs heap allocation. but maybe coalescing caller stack memory if compiler proves that coroutine lifetime nests in caller future. but on the other side, a callback style needs programmer manually manage object lifetime between caller and callback, using shated_pointer(memory allocation), coroutine the object no needs manually memory allocation, it's in coroutine heap frame, so c++ idiom RAII works well.
>

> **Q**: I'm curious, could you share what these primitives(async_mutex, sync_wait) would do (I understand the point of when_all)?
>
> **A**: To be able to create the whole application asynchronous we shouldn't block any thread from our thread pool. Traditional synchronization primitives that use os scheduler like std::mutex std::condition_variable are useless in such scenarios, we need this primitives to cooperate with internal application scheduler.
>
> **Q**: Ah, ok. Then nevermind. I was curious for my coroutine library, but mine is pure single-thread.
>
> **A**: You still need them even if you have just one thread (you just don't need to be concerned about concurrency problems) For example how you can wait for a condition and notify it from some where else in your library ? How you can join multiple asynchronous work?
>
> **A**: these primitives are needs. such a game scenario, server must await to collect all commands of clients, then continue to do game logical, this needs condition_variable.

> **Q**: Why is the epoll version slower? Shouldn't that have less overhead?
>
> **A**: maybe c code version isn't effective, and test is undulate between +-5000 rps.

> **Q**: > The result may be incredible, but it is possible, the magnitude of IO is milliseconds
>
> Depends, really. With io_uring, or user-space network stacks, you get IO in the microseconds/nanoseconds range. The best ping-pong I have seen so far with software, measured first byte in the server, first byte out, was 1.2 microseconds. This includes: the network card receiving the bytes, the PCI bus transferring them to the CPU, the CPU reading the query and writing the response, the PCI bus transferring them to the network card, the network card sending the bytes.
>
> The problem with I/O today is mostly syscall overhead, if you get syscalls out of the way -- io_uring being the most affordable alternative -- you can get an order of magnitude speed-up... or identify the next bottleneck in your code ;)
>
> **A**: If I remember correctly, a system call is about 100ns(benchmark empty epoll_wait), but io_uring may better than epoll, I see other guy compare their.

> **Q**: Why is python asyncio so performant?
>
> **A**: async program is IO bound.
>
> **A**: A lot of very expensive people have optimised the snot out of python asyncio over years. It'll always be slower in the fixed overhead sense than C++, but in terms of scalability and corner case handling it ought to be close to optimal.

> **Q**: In one example you print the call stack. Am I correct in understanding that this is the "async call stack" as opposed to the traditional call stack? And if so how did you capture this info?
>
> Im curious because this is something I've been thinking of implementing to aid debugging. Thanks.
>
> **A**: yes, it's async callstack. the point is make use of await_transform() of coroutine promise_type, that save a coroutine source_location info, in other words, when user co_await, is save await location info.(https://github.com/netcan/asyncio/blob/5ae5fdffcd065df4d9bf758741ac75647cf2f19a/include/asyncio/task.h#L113) dump backtrace is so simple, just recursive dump coroutine source_location and its continuation.

> **Q**: I'm just impressed by how readable the benchmark code looks compared to most other versions. And it seems like performance actually doesn't suffer that much from it. I wish the networking in the stdlib could look somewhat like this in practice. But it probably won't be generic enough for the committee...
>
> EDIT: It looks like you are using different buffer sizes, is there a reason behind that?
>
> **A**: the python and my project codes be written by me, others are gathered by Internet.
>
> **Q**: It would probably make sense to benchmark equal buffer sizes, since that might have an impact on the requests per second?
>
> **A**: I tested equal buffer sizes, with no impact on RPS.


## Reference
- https://github.com/lewissbaker/cppcoro
- https://docs.python.org/3/library/asyncio.html
