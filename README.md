# Felspar Coro

**C++ coroutine library and toolkit**


**felspar-coro** is a C++20 library to help you to use coroutines in your own libraries and applications. You should be able to use it with a recent clang or gcc compiler on Unix systems (e.g. Linux and Mac).


## Using the library

The library is designed to be easy to use from CMake projects. To use it add it in your source tree somewhere and then use `add_subdirectory` to include it in your project. The library also requires the use of some other libraries which can get by using an `include`. So if you have the library in a folder called `felspar-coro`, you can add the following to your `CMakeLists.txt`:

```cmake
add_subdirectory(felspar-coro)
include(felspar-coro/requirements.cmake)
```

To make use of the library add `felspar-coro` in your `target_link_libraries` directive.

```cmake
target_link_libraries(your-app otherlib1 otherlib2 felspar-coro)
```


## Coroutine functionality

A number of general coroutine awaitable types are provided, together with some useful utilities. The most important types if you want to just make use of coroutines are the `task`, the `generator` and the `stream`.


### `felspar::coro::generator`

Used to turn a coroutine that `co_yield`s values into an iterator that works with ranged `for` loops.

```cpp
std::vector<std::size_t> fibs{};
for (auto f : take(10, fib())) { fibs.push_back(f); }
check(fibs.size()) == 10u;
check(fibs.front()) == 1u;
check(fibs.back()) == 55u;
```

Generators can be used from normal functions, not just coroutines. A generator coroutine is restricted to only using `co_yield` and not `co_return`ing any value.

As well as supporting iteration, the use of `next()` directly on the generator is also supported (providing the same API structure as `stream`, but **without** the requirement to `co_await` the resulting values). This may provide a simpler API for cases where multiple generators are required to be used together.

```cpp
auto t = take(3, fib());
check(t.next()) == 1u;
check(t.next()) == 1u;
check(t.next()) == 2u;
check(t.next()).is_falsey();
```


### `felspar::coro::task`

A coroutine whose result must be awaited. Like the `lazy`, the coroutine is not started until the return value is `co_awaited`.

When used from a coroutine, either `co_await` it directly or turn the task into an r-value with `move`:

```cpp
auto val = co_await some_task();
```

```cpp
auto task = some_task();
// other things
auto val = co_await std::move(task);
```

Note that the task code will not start executing until `co_await`ed.

The task can also be started from a non-coroutine using `.get()`. This also requires a r-value, so:

```cpp
auto val some_task().get();
```

```cpp
auto task = some_task();
// other things
auto val = std::move(task).get();
```

Note that `get` will block until the coroutine completes, so it is generally used at the top level where non-coroutine code starts coroutine execution. Never use `.get()` from within a coroutine or it will likely block forever.


### `felspar::coro::stream`

A generator that must be called from a coroutine, but is allowed to use `co_await` in its internal implementation. Items are fetched from the stream one at a time using `next` and must be `co_await`ed. The returned value is a `std::optional` wrapper around the stream type and will be empty when the stream terminates.

```cpp
felspar::coro::stream<int> numbers() {
    co_yield 1;
    co_yield 2;
}

felspar::coro::stream<int> doubler(felspar::coro::stream<int> in) {
    while (auto n = co_await in.next()) {
        co_yield *n * 2;
    }
}
```


### `felspar::coro::lazy`

A basic lazily evaluated coroutine. Superficially very similar to a nullary lambda, but with an "only once" execution guarantee. The coroutine can be evaluated from either a normal function or a coroutine, and it's value is returned as if it was a nullary lambda using `operator()()`.


### `felspar::coro::start`

Starts and takes over ownership of new coroutines.


### `felspar::coro::bus`

A data bus that allows one or more coroutines to wait for a value to be produced.
