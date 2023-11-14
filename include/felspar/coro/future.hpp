#pragma once


#include <felspar/coro/coroutine.hpp>
#include <felspar/exceptions.hpp>
#include <felspar/test/source.hpp>

#include <optional>
#include <vector>


namespace felspar::coro {


    /// ## Asynchronous future
    /**
     * A non-thread safe asynchronous future. This type is not used to define a
     * coroutine (like a [task](./task.hpp) is), but is used to enable
     * communication to coroutines from other parts of the code. Typically you
     * will find this type as an instance in a data structure.
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
        std::optional<T> m_value;
        std::vector<coroutine_handle<>> continuations;


      public:
        using value_type = T;


        /// ### Query the future
        bool has_value() const noexcept { return m_value.has_value(); }
        explicit operator bool() const noexcept { return has_value(); }

        value_type &
                value(source_location const &loc = source_location::current()) {
            if (not m_value) {
                throw felspar::stdexcept::logic_error{
                        "Future does not contain a value", loc};
            } else {
                return *m_value;
            }
        }
        value_type const &value(
                source_location const &loc = source_location::current()) const {
            if (not m_value) {
                throw felspar::stdexcept::logic_error{
                        "Future does not contain a value", loc};
            } else {
                return *m_value;
            }
        }


        /// ### Coroutine interface
        auto operator co_await() {
            struct awaitable {
                coro::future<value_type> &future;
                bool await_ready() const noexcept { return future.has_value(); }
                void await_suspend(coroutine_handle<> h) {
                    future.continuations.push_back(h);
                }
                value_type &await_resume() { return *future.m_value; }
            };
            return awaitable{*this};
        }


        /// ### Set the future's value
        void set_value(
                value_type t,
                source_location const &loc = source_location::current()) {
            if (m_value) {
                throw stdexcept::logic_error{
                        "The future already has a value set", loc};
            }
            m_value = std::move(t);
            for (auto h : continuations) { h.resume(); }
            continuations = {};
        }
    };


}
