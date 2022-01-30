# Allocator support


Because every call to a coroutine involves a heap allocation for the coroutine promise, it's very important to be able to use allocators to achieve the best performance when using coroutines.


## Using `std::allocator_arg_t`

The standard library supports two mechanisms to signal that an allocator should be used during object construction:

1. Add an allocator at the end of the argument list (for example, [the `std::vector` constructors](https://en.cppreference.com/w/cpp/container/vector/vector))
2. Use tag dispatch with the allocator at beginning of the argument list. This is done by having the allocator as the second argument following an argument of type `std::allocator_arg_t` (for example, [the `std::tuple` constructors](https://en.cppreference.com/w/cpp/utility/tuple/tuple)).

Because the allocator must be used by the promise type the `operator new` for the promise type is given access to the arguments of the coroutine so that it can see the allocator. However, C++ doesn't provide any simple mechanism to find the last argument of variadic pack, so only really the second mechanism makes any sense -- it is unfortunate that it is also so ugly :-(


## Using a pre-specified allocator type

The coroutine return types (`task`, `generator` and `stream`) also support the use of a pre-arranged allocator type. This allows a custom allocator to be used when constructing the coroutine frame without the need to use `std::allocator_arg_t`.

The allocator type should be specified as the second type argument to each of them:

```
template<typename T>
using allocated_task = felspar::coro::task<T, my_allocator>;

template<typename T>
using allocated_generator = felspar::coro::generator<T, my_allocator>;

template<typename T>
using allocated_stream = felspar::coro::stream<T, my_allocator>;
```

If the first argument passed to the coroutine is an l-reference to the specified allocator type (or is a sub-class of it) then the allocator will be automatically used when allocating the coroutine stack frame. If the first argument doesn't match then the normal (global `operator new`) allocation is done.

Promise types in this library support this mechanism for all coroutine kinds through the use of `felspar::coro::promise_allocator_impl`. Specialisations (partial or full) of this type may be added for the concrete allocator types that are to be used, with the default implementation depending on a allocator type which implements `allocate` and `deallocate`.
