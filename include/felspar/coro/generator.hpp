#pragma once


#include <felspar/coro/allocator.hpp>
#include <felspar/coro/coroutine.hpp>
#include <felspar/memory/holding_pen.hpp>


namespace felspar::coro {


    template<typename Y, typename Allocator>
    struct generator_promise;


    /// A coroutine based generator. Values may be iterated (`begin`/`end`), or
    /// values may be fetched (`next`), but not both.
    template<typename Y, typename Allocator = void>
    class generator final {
        friend struct generator_promise<Y, Allocator>;
        using handle_type =
                typename generator_promise<Y, Allocator>::handle_type;
        handle_type coro;

        generator(handle_type h) : coro{std::move(h)} {}

      public:
        using promise_type = generator_promise<Y, Allocator>;

        /// Not copyable
        generator(generator const &) = delete;
        generator &operator=(generator const &) = delete;
        /// Movable
        generator(generator &&t) noexcept = default;
        generator &operator=(generator &&t) noexcept = default;
        ~generator() = default;

        /// Iteration
        class iterator {
            friend class generator;
            handle_type coro;

            iterator() : coro{} {}
            iterator(generator *s) : coro{std::move(s->coro)} {
                coro.resume();
                throw_if_needed();
            }

            void throw_if_needed() {
                if (coro.promise().eptr) {
                    std::rethrow_exception(coro.promise().eptr);
                }
            }

          public:
            iterator(iterator const &) = delete;
            iterator(iterator &&i) = default;
            iterator &operator=(iterator const &) = delete;
            iterator &operator=(iterator &&i) = default;
            ~iterator() = default;

            Y operator*() {
                return static_cast<Y &&>(
                        std::move(coro.promise().value).transfer_out().value());
            }

            auto &operator++() {
                coro.resume();
                throw_if_needed();
                if (not coro.promise().value) { coro = {}; }
                return *this;
            }

            friend bool operator==(iterator const &l, iterator const &r) {
                if (not l.coro && not r.coro) {
                    return true;
                } else if (l.coro && l.coro.done() && not r.coro) {
                    return true;
                } else if (not l.coro && r.coro && r.coro.done()) {
                    return true;
                } else {
                    return false;
                }
            }
        };
        auto begin() { return iterator{this}; }
        auto end() { return iterator{}; }

        /// Fetching values. Returns an empty `optional` when completed.
        memory::holding_pen<Y> next() {
            coro.resume();
            if (coro.promise().eptr) {
                std::rethrow_exception(coro.promise().eptr);
            } else {
                return std::move(coro.promise().value).transfer_out();
            }
        }
    };


    template<typename Y, typename Allocator>
    struct generator_promise : private promise_allocator_impl<Allocator> {
        using promise_allocator_impl<Allocator>::operator new;
        using promise_allocator_impl<Allocator>::operator delete;

        memory::holding_pen<Y> value = {};
        std::exception_ptr eptr = {};

        using handle_type = unique_handle<generator_promise>;

        template<typename A>
        std::suspend_always await_transform(A &&) = delete; // Use stream

        auto yield_value(Y y) {
            value.emplace(std::move(y));
            return std::suspend_always{};
        }
        void unhandled_exception() { eptr = std::current_exception(); }

        auto return_void() {
            value.reset();
            return std::suspend_never{};
        }

        auto get_return_object() {
            return generator<Y, Allocator>{handle_type::from_promise(*this)};
        }
        auto initial_suspend() const noexcept { return std::suspend_always{}; }
        auto final_suspend() const noexcept { return std::suspend_always{}; }
    };


}
