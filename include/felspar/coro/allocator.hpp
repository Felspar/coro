#pragma once


#include <felspar/memory/any_buffer.hpp>
#include <cstddef>
#include <stdexcept>


namespace felspar::coro {


    /// Allocator implementation for
    template<typename Allocator>
    struct promise_allocator_impl;


    template<>
    struct promise_allocator_impl<void> {
        struct allocation {
            /// Delete the promise
            void (*promise_deleter)(void *, void *);
            /// Store allocation details
            void *allocator = nullptr;
        };

        /// Calculate the lowest offset for an aligned memory block above the
        /// base offset
        static std::size_t aligned_offset(std::size_t const base) {
            std::size_t const alignment = alignof(allocation);
            return (base + alignment - 1) & ~(alignment - 1);
        }

        void *operator new(std::size_t const psize) {
            auto const alloc_base = aligned_offset(psize);
            auto const size = alloc_base + sizeof(allocation);
            std::byte *base{
                    reinterpret_cast<std::byte *>(::operator new(size))};
            new (base + alloc_base) allocation{
                    [](void *, void *ptr) { ::operator delete(ptr); }};
            return base;
        }
        void operator delete(void *const ptr, std::size_t const psize) {
            auto const alloc_base = aligned_offset(psize);
            std::byte *const base = reinterpret_cast<std::byte *>(ptr);
            allocation *palloc = std::launder(
                    reinterpret_cast<allocation *>(base + alloc_base));
            palloc->promise_deleter(palloc->allocator, ptr);
        }
    };


    template<typename Allocator>
    struct promise_allocator_impl : public promise_allocator_impl<void> {
        using promise_allocator_impl<void>::operator new;
        using promise_allocator_impl<void>::operator delete;

        template<typename... Args>
        void *operator new(
                std::size_t const psize, Allocator &alloc, Args &&...) {
            auto const alloc_base = aligned_offset(psize);
            auto const size = alloc_base + sizeof(allocation);
            std::unique_ptr<std::byte> base{alloc.allocate(size)};
            new (base.get() + alloc_base) allocation{
                    [](void *b, void *ptr) {
                        auto *palloc = reinterpret_cast<Allocator *>(b);
                        palloc->deallocate(reinterpret_cast<std::byte *>(ptr));
                    },
                    &alloc};
            return base.release();
        }
    };


}
