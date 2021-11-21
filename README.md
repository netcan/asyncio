# asyncio
Asyncio is a C++20 coroutine library to write concurrent code using the await syntax.

It's for study purpose, to learn C++20 coroutine mechanism, and to imitate the python asyncio interfaces.

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
