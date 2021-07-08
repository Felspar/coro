#pragma once


#include <felspar/coro/coroutine.hpp>

#include <optional>
#include <stdexcept>


namespace felspar::coro {


    template<typename Y>
    class task;
    template<typename Y>
    struct task_promise;


    struct task_promise_base {
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
            return symmetric_continuation{continuation};
        }
    };
    template<>
    struct task_promise<void> final : public task_promise_base {
        using value_type = void;
        using handle_type = unique_handle<task_promise<void>>;

        task<void> get_return_object();

        bool has_value = false;
        void return_void() noexcept { has_value = true; }
        void consume_value() {
            check_exception();
            if (not has_value) {
                throw std::runtime_error{"The task hasn't completed"};
            }
        }
    };
    template<typename Y>
    struct task_promise final : public task_promise_base {
        using value_type = Y;
        using handle_type = unique_handle<task_promise<Y>>;

        task<value_type> get_return_object();

        std::optional<value_type> value = {};
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


    template<typename Y>
    class [[nodiscard]] task final {
        friend struct task_promise<Y>;

      public:
        using value_type = Y;
        using promise_type = task_promise<value_type>;
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
                bool await_ready() const noexcept { return false; }
                coroutine_handle<>
                        await_suspend(coroutine_handle<> awaiting) noexcept {
                    coro.promise().continuation = awaiting;
                    if (not coro.promise().started) {
                        coro.promise().started = true;
                        return coro.get();
                    } else {
                        return noop_coroutine();
                    }
                }
                Y await_resume() { return coro.promise().consume_value(); }
            };
            return awaitable{std::move(coro)};
        }

        /// Or use this from a normal function
        value_type get() & = delete;
        value_type get() && {
            if (not coro) {
                throw std::runtime_error{"No coroutine to resume"};
            }
            start();
            return coro.promise().consume_value();
        }

        /// Or take on responsibility for the coroutine
        handle_type release() {
            auto c = std::move(coro);
            return c;
        }

      private:
        handle_type coro;

        void start() {
            if (not coro) {
                throw std::runtime_error{"Cannot start an empty task"};
            } else if (not coro.promise().started) {
                coro.promise().started = true;
                coro.resume();
            }
        }
    };


    inline task<void> task_promise<void>::get_return_object() {
        return task<void>{handle_type::from_promise(*this)};
    }
    template<typename T>
    inline auto task_promise<T>::get_return_object() -> task<value_type> {
        return task<value_type>{handle_type::from_promise(*this)};
    }


}
