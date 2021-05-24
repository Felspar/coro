#pragma once


#include <felspar/coro/coroutine.hpp>
#include <felspar/memory/holding_pen.hpp>


namespace felspar::coro {


    template<typename Y>
    struct stream_promise;


    template<typename Y>
    class stream final {
        friend struct stream_promise<Y>;
        using handle_type = typename stream_promise<Y>::handle_type;
        handle_type yielding_coro;

        stream(handle_type h) : yielding_coro{std::move(h)} {}

      public:
        using promise_type = stream_promise<Y>;

        /// Not copyable
        stream(stream const &) = delete;
        stream &operator=(stream const &) = delete;
        /// Movable
        stream(stream &&t) noexcept = default;
        stream &operator=(stream &&t) noexcept = default;
        ~stream() = default;

        auto next() {
            struct awaitable {
                handle_type &yielding_coro;
                bool await_ready() const noexcept {
                    return yielding_coro.promise().completed;
                }
                auto await_suspend(coroutine_handle<> awaiting) noexcept {
                    yielding_coro.promise().continuation = awaiting;
                    return yielding_coro.get();
                }
                memory::holding_pen<Y> await_resume() {
                    if (auto eptr = yielding_coro.promise().eptr) {
                        std::rethrow_exception(eptr);
                    } else {
                        return std::move(yielding_coro.promise().value)
                                .transfer_out();
                    }
                }
            };
            return awaitable{yielding_coro};
        }
    };


    template<typename Y>
    struct stream_promise {
        coroutine_handle<> continuation = {};
        bool completed = false;
        memory::holding_pen<Y> value = {};
        std::exception_ptr eptr = {};

        using handle_type = unique_handle<stream_promise>;

        auto yield_value(Y y) {
            value.assign(std::move(y));
            return symmetric_continuation{std::exchange(continuation, {})};
        }

        void unhandled_exception() { eptr = std::current_exception(); }

        auto return_void() {
            completed = true;
            value.reset();
            return suspend_never{};
        }

        auto get_return_object() {
            return stream<Y>{handle_type::from_promise(*this)};
        }

        auto initial_suspend() const noexcept { return suspend_always{}; }
        auto final_suspend() const noexcept {
            return symmetric_continuation{continuation};
        }
    };


    template<typename Y, typename X>
    inline X operator|(stream<Y> &&s, X (*c)(stream<Y>)) {
        return c(std::move(s));
    }


}
