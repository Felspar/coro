#pragma once


#include <felspar/memory/sizes.hpp>

#include <new>
#include <utility>


namespace felspar::coro {


    /// Allocator implementation for
    template<typename Allocator>
    struct promise_allocator_impl;


    template<>
    struct promise_allocator_impl<void> {
        void *operator new(std::size_t sz) { return ::operator new(sz); }
        void operator delete(void *const ptr) { return ::operator delete(ptr); }
        void operator delete(void *const ptr, std::size_t) {
            return ::operator delete(ptr);
        }
    };


    template<typename Allocator>
    struct promise_allocator_impl {
        struct allocation {
            /// ### Size of the allocation
            /// Includes the size of this structure and padding
            std::size_t total_size;
            /// ### Store allocator details
            Allocator *allocator = nullptr;
        };

        static constexpr std::size_t allocator_block_size =
                felspar::memory::block_size(
                        sizeof(allocation), alignof(std::max_align_t));

        void *operator new(std::size_t const psize) {
            std::size_t const allocation_size = allocator_block_size + psize;
            std::byte *base{reinterpret_cast<std::byte *>(
                    ::operator new(allocation_size))};
            new (base) allocation{allocation_size};
            return base + allocator_block_size;
        }

        /// ### Deal with functions that are passed allocators
        template<typename... Args>
        void *operator new(std::size_t const psize, Allocator &alloc, Args &...) {
            std::size_t const allocation_size = allocator_block_size + psize;
            std::byte *base{reinterpret_cast<std::byte *>(
                    alloc.allocate(allocation_size))};
            new (base) allocation{allocation_size, &alloc};
            return base + allocator_block_size;
        }
        /// ### Deal with members that are coroutines
        /// TODO `This` should almost certainly be much more restrictive than it
        /// is here
        template<typename This, typename... Args>
        void *operator new(
                std::size_t const sz, This &&, Allocator &alloc, Args &...) {
            return operator new(sz, alloc);
        }

        void operator delete(void *const ptr) {
            auto const frame = reinterpret_cast<std::byte *>(ptr);
            auto const base_ptr = reinterpret_cast<allocation *>(
                    frame - allocator_block_size);
            auto const details = std::move(*base_ptr);
            base_ptr->~allocation();
            if (details.allocator) {
                details.allocator->deallocate(base_ptr, details.total_size);
            } else {
                ::operator delete(base_ptr);
            }
        }
        void operator delete(void *const ptr, std::size_t) {
            operator delete(ptr);
        }
    };


}
