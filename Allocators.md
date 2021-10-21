# Allocator support


Because every call to a coroutine involves a heap allocation for the coroutine promise it's very important to be able to use allocators for achieve the best performance when using coroutines.


## Using `std::allocator_arg_t`

The standard library uses two mechanisms to signal that an allocator should be used:

1. Add an allocator at the end of the argument list (for example, [the `std::vector` constructors](https://en.cppreference.com/w/cpp/container/vector/vector))
2. Use tag dispatch with the allocator at beginning of the argument list. This is done by having the allocator as the second argument following an argument of type `std::allocator_arg_t` (for example, [the `std::tuple` constructors](https://en.cppreference.com/w/cpp/utility/tuple/tuple)).

Because the allocator must be used by the promise type the `operator new` for the promise type is given access to the arguments of the coroutine so that it can see the allocator. However, C++ doesn't provide any simple mechanism to find the last argument of variadic pack, so only really the second mechanism makes any sense -- it is unfortunate that it is also so ugly :-(

Promise types in this library support this second mechanism for all coroutine kinds through the use of `felspar::coro::promise_allocator_impl`.

