#pragma once


#include <felspar/coro/coroutine.hpp>

#include <exception>
#include <optional>


namespace felspar::coro {


    template<typename L>
    class lazy {
      public:
        lazy(lazy const &) = delete;
        lazy &operator=(lazy const &) = delete;
        lazy(lazy &&o) = default;
        lazy &operator=(lazy &&o) = default;
        ~lazy() = default;

        struct promise_type {
            std::exception_ptr eptr;
            std::optional<L> value;
            using handle_type = unique_handle<promise_type>;

            lazy get_return_object() {
                return {handle_type::from_promise(*this)};
            }

            template<typename A>
            suspend_always await_transform(A &&) = delete; // Use async

            void unhandled_exception() { eptr = std::current_exception(); }
            void return_value(L v) { value = std::move(v); }

            auto initial_suspend() const noexcept { return suspend_always{}; }
            auto final_suspend() const noexcept { return suspend_always{}; }
        };
        friend promise_type;

        L operator()() {
            if (not coro.done()) { coro.resume(); }
            if (coro.promise().eptr) {
                std::rethrow_exception(coro.promise().eptr);
            } else {
                return *coro.promise().value;
            }
        }

      private:
        using handle_type = typename promise_type::handle_type;
        lazy(handle_type h) : coro{std::move(h)} {}
        handle_type coro;
    };


}
