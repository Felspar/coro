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
