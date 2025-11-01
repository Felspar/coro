#pragma once


#include <felspar/coro/allocator.hpp>
#include <felspar/coro/coroutine.hpp>
#include <felspar/coro/forward.hpp>
#include <felspar/exceptions.hpp>

#include <exception>
#include <optional>
#include <stdexcept>


namespace felspar::coro {


    template<typename Allocator>
    struct task_promise_base : private promise_allocator_impl<Allocator> {
        using promise_allocator_impl<Allocator>::operator new;
        using promise_allocator_impl<Allocator>::operator delete;

        /// Flag to ensure the coroutine is started at appropriate points in time
        bool started = false;
        /// Any caught exception that needs to be re-thrown is captured here
        std::exception_ptr eptr = {};
        void check_exception() {
            if (eptr) { std::rethrow_exception(eptr); }
        }
        /// The continuation that is to run when the task is complete
        std::coroutine_handle<> continuation = {};

        auto initial_suspend() const noexcept { return std::suspend_always{}; }
        void unhandled_exception() noexcept { eptr = std::current_exception(); }
        auto final_suspend() noexcept {
            return symmetric_continuation{std::exchange(continuation, {})};
        }
    };
    template<typename Allocator>
    struct task_promise<void, Allocator> final :
    public task_promise_base<Allocator> {
        using value_type = void;
        using allocator_type = Allocator;
        using unique_handle_type = unique_handle<task_promise>;

        using task_promise_base<allocator_type>::eptr;
        using task_promise_base<allocator_type>::check_exception;

        task<void, allocator_type> get_return_object();

        bool has_returned = false;
        bool has_value() const noexcept { return has_returned or eptr; }
        void return_void() noexcept { has_returned = true; }

        void consume_value() {
            check_exception();
            if (not has_returned) {
                throw std::runtime_error{"The task hasn't completed"};
            }
        }
    };
    template<typename Y, typename Allocator>
    struct task_promise final : public task_promise_base<Allocator> {
        using value_type = Y;
        using allocator_type = Allocator;
        using unique_handle_type = unique_handle<task_promise>;

        using task_promise_base<allocator_type>::eptr;
        using task_promise_base<allocator_type>::check_exception;

        task<value_type, allocator_type> get_return_object();

        std::optional<value_type> value = {};
        bool has_value() const noexcept { return value or eptr; }
        void return_value(value_type y) { value = std::move(y); }

        FELSPAR_CORO_WRAPPER value_type consume_value() {
            check_exception();
            if (not value.has_value()) {
                throw stdexcept::runtime_error{
                        "The task hasn't completed with a value "};
            }
            value_type rv = std::move(*value);
            value.reset();
            return rv;
        }
    };


    /// ## Tasks
    template<typename Y, typename Allocator>
    class [[nodiscard]] FELSPAR_CORO_CRT task final {
        friend class eager<task>;
        friend class starter<task>;
        friend struct task_promise<Y, Allocator>;


      public:
        using value_type = Y;
        using allocator_type = Allocator;
        using promise_type = task_promise<value_type, allocator_type>;
        using unique_handle_type = typename promise_type::unique_handle_type;


        /// ### Construction

        /// #### Construct a new task from a previously released one
        explicit task(unique_handle_type h) : coro{std::move(h)} {}


        /// #### Not copyable
        task(task const &) = delete;
        task &operator=(task const &) = delete;
        /// #### Movable
        task(task &&t) noexcept = default;
        task &operator=(task &&t) noexcept = default;
        ~task() = default;


        /// ### Awaitable
        auto operator co_await() & = delete;
        FELSPAR_CORO_WRAPPER auto operator co_await() && {
            /**
             * The awaitable takes over ownership of the coroutine handle once
             * its been created. This ensures that the lifetime of the promise
             * is long enough to deliver the return value.
             */
            struct FELSPAR_CORO_CRT awaitable {
                unique_handle_type coro;
                ~awaitable() { coro.promise().continuation = {}; }

                bool await_ready() const noexcept {
                    return coro.promise().has_value();
                }
                std::coroutine_handle<> await_suspend(
                        std::coroutine_handle<> awaiting) noexcept {
                    if (not coro.promise().started) {
                        coro.promise().continuation = awaiting;
                        coro.promise().started = true;
                        return coro.get();
                    } else if (coro.promise().has_value()) {
                        return awaiting;
                    } else {
                        coro.promise().continuation = awaiting;
                        return std::noop_coroutine();
                    }
                }
                FELSPAR_CORO_WRAPPER Y await_resume() {
                    return coro.promise().consume_value();
                }
            };
            return awaitable{.coro = std::move(coro)};
        }


        /// ### Or use this from a normal function
        value_type get() & = delete;
        value_type get(
                source_location const &loc = source_location::current()) && {
            start(loc);
            return coro.promise().consume_value();
        }


        /// ### Or take on responsibility for the coroutine
        unique_handle_type release() { return {std::move(coro)}; }


      private:
        unique_handle_type coro;

        void start(source_location const &loc = source_location::current()) {
            if (not coro) {
                throw stdexcept::runtime_error{
                        "Cannot start an empty task", loc};
            } else if (not coro.promise().started) {
                coro.promise().started = true;
                coro.resume();
            }
        }
    };


    template<typename Allocator>
    inline auto task_promise<void, Allocator>::get_return_object()
            -> task<void, allocator_type> {
        return task<void, allocator_type>{
                unique_handle_type::from_promise(*this)};
    }
    template<typename T, typename Allocator>
    inline auto task_promise<T, Allocator>::get_return_object()
            -> task<value_type, allocator_type> {
        return task<value_type, allocator_type>{
                unique_handle_type::from_promise(*this)};
    }


}
