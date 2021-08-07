# Prime numbers

The prime number generator examples various implementations of the Sieve of Eratosthenes to find prime numbers up to a numeric limit. For a detailed description of how the sieve works see [primes-1-generator.cpp](./primes-1-generator.cpp).

The times are recorded on my desktop (AMD R9 5950X) without really controlling for thermals or boost clocks, so they're a bit noisy, but the differences are large enough that the differences are clear anyway.

Because the sieve only requires addition and comparison, it should make this quite a good benchmark for the underlying coroutine machinery and the compiler's ability to optimise them.


## primes-1-generator.cpp

[This is a very simple and straight forward implementation](./primes-1-generator.cpp) of the idea. It's simple enough that it didn't require any debugging -- it simply worked first time.

*  for up to 1 million
  - clang 14s
  - gcc core dump at about 820,000

As you push clang to looking at larger numbers it also dumps core. The core dump is due to stack exhaustion. Basically the `generator` has to call in to the one below in order to pull the next value. Eventually the stack of coroutines in the sieve becomes large enough that they cause a stack overflow.


## primes-2-stream.cpp

[This just changes the generator type](./primes-2-stream.cpp) to `stream` (an asynchronous generator type) and adds the required `co_await`. Now we also need to lift the old `main` into a coroutine, which can be easily done using `task`.

* for up to 1 million
  - clang 36s
  - gcc 33s

This implementation is clearly a good deal slower, but has the advantage that it can be pushed to much larger values due to the fact that the stream together with the `co_await` allows for symmetric transfer when values are fetched from further down in the sieve. This means that this version never needs more than a handful of stack frames to work so the sieve size is limited only by available RAM and not by available stack.


## primes-3-optimised.cpp

The straight forward implementations above always added all primes to the sieve, but it isn't necessary to add the prime until the number we're wanting to check is larger than the primes square. [This version implements that](./primes-3-optimised.cpp) and does it in such a way that it preserves the "only use addition and comparisons" rule, meaning it keeps track of the squares without using multiplication.

* for up to 1 million
  - clang 0.2s
  - gcc 0.15s
* for up to 694,183,367
  - clang 16m20s
  - gcc 16m17s

The speed up is impressive, and allows us to check much larger numbers with reasonable cost.


## Conclusions

It seems that clang is more efficient with the stack usage, but gcc is slightly better at optimising the code. At up to 10-25% improvement this is a significant win for gcc though.

It's also clear that the `generator` type is generally much faster than the `stream`, but the inability to do real symmetric transfer (the continuation must be called directly) means that you must be careful about stack usage if you have very long chains of them. The `stream` type is far more scalable, but it comes with the cost that it can only be used from coroutines due to the required `co_await`.

The optimised implementation just goes to show that micro-optimisations are great and all, but looking for more efficient algorithms is a far better use of time.
