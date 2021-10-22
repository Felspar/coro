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
        void *operator new(
                std::size_t const psize, Allocator &alloc, Args &&...) {
            auto const alloc_base = aligned_offset(psize);
            auto const size = alloc_base + sizeof(allocation);
            std::byte *base{alloc.allocate(size)};
            new (base + alloc_base) allocation{&alloc};
            return base;
        }
        void operator delete(void *const ptr, std::size_t const psize) {
            auto const alloc_base = aligned_offset(psize);
            std::byte *const base = reinterpret_cast<std::byte *>(ptr);
            allocation *palloc =
                    reinterpret_cast<allocation *>(base + alloc_base);
            if (palloc->allocator) {
                auto *alloc = palloc->allocator;
                alloc->deallocate(reinterpret_cast<std::byte *>(ptr));
            } else {
                ::operator delete(ptr);
            }
        }
    };


}
