#pragma once


#include <felspar/coro/coroutine.hpp>
#include <felspar/exceptions.hpp>

#include <optional>
#include <vector>


namespace felspar::coro {


    /// ## Asynchronous future
    /**
     * An nopn-thread safe asynchronous future.
     *
     * It is able to have multiple coroutines wait on the future, with them all
     * being resumed when a value is pushed. If the value has already been set
     * then any new coroutine will continue without suspending.
     *
     * The value can be set and read from non-coroutines as well. The value can
     * only be set once.
     */
    template<typename T>
    class future {
        std::optional<T> value;
        std::vector<coroutine_handle<>> continuations;


      public:
        using value_type = T;


        /// ### Query the future
        bool has_value() const noexcept { return value.has_value(); }
        explicit operator bool() const noexcept { return has_value(); }


        /// ### Coroutine interface
        auto operator co_await() {
            struct awaitable {
                coro::future<value_type> &future;
                bool await_ready() const noexcept { return future.has_value(); }
                void await_suspend(coroutine_handle<> h) {
                    future.continuations.push_back(h);
                }
                value_type &await_resume() { return *future.value; }
            };
            return awaitable{*this};
        }


        /// ### Set the future's value
        void set_value(value_type t) {
            if (value) {
                throw stdexcept::logic_error{
                        "The future already has a value set"};
            }
            value = std::move(t);
            for (auto h : continuations) { h.resume(); }
            continuations = {};
        }
    };


}
