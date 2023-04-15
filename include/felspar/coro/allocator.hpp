#pragma once


#include <cstddef>
#include <new>


namespace felspar::coro {


    /// Allocator implementation for
    template<typename Allocator>
    struct promise_allocator_impl;


    template<>
    struct promise_allocator_impl<void> {
        void *operator new(std::size_t sz) { return ::operator new(sz); }
        void operator delete(void *const ptr, std::size_t) {
            return ::operator delete(ptr);
        }
    };


    template<typename Allocator>
    struct promise_allocator_impl {
        struct allocation {
            /// Store allocation details
            Allocator *allocator = nullptr;
        };

        // TODO This should be in felspar-memory
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
            new (base + alloc_base) allocation{};
            return base;
        }

        template<typename... Args>
        void *operator new(std::size_t const psize, Allocator &alloc, Args &...) {
            auto const alloc_base = aligned_offset(psize);
            auto const size = alloc_base + sizeof(allocation);
            std::byte *base{
                    reinterpret_cast<std::byte *>(alloc.allocate(size))};
            new (base + alloc_base) allocation{&alloc};
            return base;
        }
        /// Deal with members that are coroutines. `This` should almost
        /// certainly be much more restrictive than it is here
        template<typename This, typename... Args>
        void *operator new(
                std::size_t const sz, This &&, Allocator &alloc, Args &...) {
            return operator new(sz, alloc);
        }

        void operator delete(void *const ptr, std::size_t const psize) {
            auto const alloc_base = aligned_offset(psize);
            std::byte *const base = reinterpret_cast<std::byte *>(ptr);
            allocation *palloc = std::launder(
                    reinterpret_cast<allocation *>(base + alloc_base));
            if (palloc->allocator) {
                auto const size = alloc_base + sizeof(allocation);
                auto *alloc = palloc->allocator;
                alloc->deallocate(ptr, size);
            } else {
                ::operator delete(ptr);
            }
        }
    };


}
