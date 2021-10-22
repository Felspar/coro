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
            void (*promise_deleter)(felspar::memory::any_buffer &, void *);
            /// Store allocation details
            felspar::memory::any_buffer allocator;
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
            std::unique_ptr<std::byte> base{::new std::byte[size]};
            new (base.get() + alloc_base)
                    allocation{[](felspar::memory::any_buffer &, void *ptr) {
                        ::delete[](reinterpret_cast<std::byte *>(ptr));
                    }};
            return base.release();
        }
        void operator delete(void *const ptr, std::size_t const psize) {
            auto const alloc_base = aligned_offset(psize);
            std::byte *const base = reinterpret_cast<std::byte *>(ptr);
            allocation *palloc = std::launder(
                    reinterpret_cast<allocation *>(base + alloc_base));
            auto alloc = std::move(*palloc);
            std::destroy_at(palloc);
            alloc.promise_deleter(alloc.allocator, ptr);
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
                    [](felspar::memory::any_buffer &b, void *ptr) {
                        auto palloc = b.fetch<Allocator *>();
                        (*palloc)->deallocate(
                                reinterpret_cast<std::byte *>(ptr));
                    },
                    &alloc};
            return base.release();
        }
    };


}
