/**
    Copyright 2019-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once

#include <felspar/coro/coroutine.hpp>
#include <optional>


namespace felspar::coro {


    template<typename Y>
    struct generator_promise;


    /// A coroutine based generator. Values may be iterated (`begin`/`end`), or
    /// values may be fetched (`next`), but not both.
    template<typename Y>
    class generator final {
        friend struct generator_promise<Y>;
        using handle_type = typename generator_promise<Y>::handle_type;
        handle_type coro;

        generator(handle_type h) : coro{std::move(h)} {}

      public:
        using promise_type = generator_promise<Y>;

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

            Y operator*() { return *std::exchange(coro.promise().value, {}); }

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
        std::optional<Y> next() {
            coro.resume();
            if (coro.promise().eptr) {
                std::rethrow_exception(coro.promise().eptr);
            }
            return std::exchange(coro.promise().value, {});
        }
    };


    template<typename Y>
    struct generator_promise {
        std::optional<Y> value = {};
        std::exception_ptr eptr = {};

        using handle_type = unique_handle<generator_promise>;

        template<typename A>
        suspend_always await_transform(A &&) = delete; // Use stream

        auto yield_value(Y y) {
            value = std::move(y);
            return suspend_always{};
        }
        void unhandled_exception() { eptr = std::current_exception(); }

        auto return_void() {
            value = {};
            return suspend_never{};
        }

        auto get_return_object() {
            return generator<Y>{handle_type::from_promise(*this)};
        }
        auto initial_suspend() const noexcept { return suspend_always{}; }
        auto final_suspend() const noexcept { return suspend_always{}; }
    };


}
