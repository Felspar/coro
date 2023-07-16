#pragma once


#include <felspar/coro/allocator.hpp>
#include <felspar/coro/coroutine.hpp>
#include <felspar/exceptions.hpp>

#include <optional>
#include <stdexcept>


namespace felspar::coro {


    template<typename Task>
    class eager;
    template<typename Task>
    class starter;
    template<typename Y, typename Allocator = void>
    class task;
    template<typename Y, typename Allocator>
    struct task_promise;


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
        coroutine_handle<> continuation = {};

        auto initial_suspend() const noexcept { return suspend_always{}; }
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
        using handle_type = unique_handle<task_promise>;

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
        using handle_type = unique_handle<task_promise>;

        using task_promise_base<allocator_type>::eptr;
        using task_promise_base<allocator_type>::check_exception;

        task<value_type, allocator_type> get_return_object();

        std::optional<value_type> value = {};
        bool has_value() const noexcept { return value or eptr; }
        void return_value(value_type y) { value = std::move(y); }

        value_type consume_value() {
            check_exception();
            if (not value.has_value()) {
                throw std::runtime_error{
                        "The task hasn't completed with a value "};
            }
            value_type rv = std::move(*value);
            value.reset();
            return rv;
        }
    };


    template<typename Y, typename Allocator>
    class [[nodiscard]] task final {
        friend class eager<task>;
        friend class starter<task>;
        friend struct task_promise<Y, Allocator>;

      public:
        using value_type = Y;
        using allocator_type = Allocator;
        using promise_type = task_promise<value_type, allocator_type>;
        using handle_type = typename promise_type::handle_type;

        /// Construct a new task from a previously released one
        explicit task(handle_type h) : coro{std::move(h)} {}

        /// Not copyable
        task(task const &) = delete;
        task &operator=(task const &) = delete;
        /// Movable
        task(task &&t) noexcept = default;
        task &operator=(task &&t) noexcept = default;
        ~task() = default;

        /// Coroutine and awaitable
        auto operator co_await() & = delete;
        auto operator co_await() && {
            struct awaitable {
                handle_type coro;
                bool await_ready() const noexcept {
                    return coro.promise().has_value();
                }
                coroutine_handle<>
                        await_suspend(coroutine_handle<> awaiting) noexcept {
                    if (not coro.promise().started) {
                        coro.promise().continuation = awaiting;
                        coro.promise().started = true;
                        return coro.get();
                    } else if (coro.promise().has_value()) {
                        return awaiting;
                    } else {
                        coro.promise().continuation = awaiting;
                        return noop_coroutine();
                    }
                }
                Y await_resume() { return coro.promise().consume_value(); }
            };
            return awaitable{std::move(coro)};
        }

        /// Or use this from a normal function
        value_type get() & = delete;
        value_type get(
                source_location const &loc = source_location::current()) && {
            start(loc);
            return coro.promise().consume_value();
        }

        /// Or take on responsibility for the coroutine
        handle_type release() {
            auto c = std::move(coro);
            return c;
        }

      private:
        handle_type coro;

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
        return task<void, allocator_type>{handle_type::from_promise(*this)};
    }
    template<typename T, typename Allocator>
    inline auto task_promise<T, Allocator>::get_return_object()
            -> task<value_type, allocator_type> {
        return task<value_type, allocator_type>{
                handle_type::from_promise(*this)};
    }


}
