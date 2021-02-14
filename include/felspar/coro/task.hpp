#pragma once


#include <felspar/coro/coroutine.hpp>

#include <optional>
#include <stdexcept>


namespace felspar::coro {


    template<typename Y>
    struct task_promise;


    template<typename Y>
    class [[nodiscard]] task final {
        friend struct task_promise<Y>;

      public:
        using handle_type = typename task_promise<Y>::handle_type;

        /// Not copyable
        task(task const &) = delete;
        task &operator=(task const &) = delete;
        /// Movable
        task(task &&t) noexcept = default;
        task &operator=(task &&t) noexcept = default;
        /// TODO An asynchronous task may get to here while still be alive
        /// blocked on whatever asynchronous API it is waiting on. In this
        /// case calling destroy will be a big problem for it. The coroutine
        /// here should see that and decouple and leave it to the
        /// asynchronous mechanism to destroy the coroutine.
        ~task() = default;

        void start() {
            if (not started) {
                started = true;
                coro.resume();
            }
        }

        /// Coroutine and awaitable
        using promise_type = task_promise<Y>;

        auto operator co_await() & = delete;
        auto operator co_await() && {
            struct awaitable {
                task &t;
                bool await_ready() const noexcept { return false; }
                coroutine_handle<>
                        await_suspend(coroutine_handle<> awaiting) noexcept {
                    t.coro.promise().continuation = awaiting;
                    if (not t.started) {
                        t.started = true;
                        return t.coro.get();
                    } else {
                        return noop_coroutine();
                    }
                }
                Y await_resume() { return t.coro.promise().consume_value(); }
            };
            return awaitable{*this};
        }

        /// Or use this from a normal function
        Y get() & = delete;
        Y get() && {
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
        bool started = false;
        explicit task(handle_type h) : coro{std::move(h)} {}
    };


    struct task_promise_base {
        std::exception_ptr eptr = {};
        coroutine_handle<> continuation = {};
        auto initial_suspend() const noexcept { return suspend_always{}; }
        void unhandled_exception() noexcept { eptr = std::current_exception(); }
        auto final_suspend() noexcept {
            return symmetric_continuation{continuation};
        }
        void consume_value() {
            if (eptr) { std::rethrow_exception(eptr); }
        }
    };
    template<>
    struct task_promise<void> : public task_promise_base {
        using handle_type = unique_handle<task_promise<void>>;
        auto get_return_object() {
            return task<void>{handle_type::from_promise(*this)};
        }
        void return_void() const noexcept {}
    };
    template<typename Y>
    struct task_promise : public task_promise_base {
        using handle_type = unique_handle<task_promise<Y>>;
        std::optional<Y> value = {};
        auto get_return_object() {
            return task<Y>{handle_type::from_promise(*this)};
        }
        void return_value(Y y) { value = std::move(y); }
        Y consume_value() {
            task_promise_base::consume_value();
            if (not value.has_value()) {
                throw std::runtime_error("The task doesn't have a value");
            }
            Y rv = std::move(*value);
            value.reset();
            return rv;
        }
    };


}
