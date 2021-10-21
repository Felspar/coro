#pragma once


#include <cstddef>
#include <stdexcept>


namespace felspar::coro {


    /// Allocator implementation for
    template<typename Allocator>
    struct promise_allocator_impl;


    template<>
    struct promise_allocator_impl<void> {
        void *operator new(std::size_t const size) {
            return ::operator new(size);
        }
        void operator delete(void *const ptr, std::size_t const size) {
            return ::operator delete(ptr);
        }
    };


    template<typename Allocator>
    struct promise_allocator_impl : public promise_allocator_impl<void> {
        using promise_allocator_impl<void>::operator new;
        using promise_allocator_impl<void>::operator delete;
    };


}
