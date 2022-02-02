# Felspar Coro

**C++ coroutine library and toolkit**


## `felspar::coro::generator`

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


## `felspar::coro::task`

A coroutine whose result must be awaited. Like the `lazy`, the coroutine is not started until the return value is `co_awaited`.

When used from a coroutine, either `co_await` it directly or turn the task into an r-value with move:

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


## `felspar::coro::stream`

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


## `felspar::coro::lazy`

A basic lazily evaluated coroutine. Superficially very similar to a nullary lambda, but with an "only once" execution guarantee. The coroutine can be evaluated from either a normal function or a coroutine, and it's value is returned as if it was a nullary lambda using `operator()()`.


## `felspar::coro::start`

Starts and takes over ownership of new coroutines.
