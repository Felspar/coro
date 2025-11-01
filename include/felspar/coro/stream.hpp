#pragma once


#include <felspar/coro/allocator.hpp>
#include <felspar/coro/coroutine.hpp>
#include <felspar/memory/holding_pen.hpp>

#include <exception>


namespace felspar::coro {


    template<typename Y, typename H>
    class stream_awaitable;
    template<typename Y, typename Allocator>
    struct stream_promise;


    template<typename Y, typename Allocator = void>
    class FELSPAR_CORO_CRT stream final {
        friend struct stream_promise<Y, Allocator>;
        using handle_type = typename stream_promise<Y, Allocator>::handle_type;
        handle_type yielding_coro;

        stream(handle_type h) : yielding_coro{std::move(h)} {}

      public:
        using value_type = Y;
        using optional_type = memory::holding_pen<value_type>;
        using promise_type = stream_promise<value_type, Allocator>;

        /// Not copyable
        stream(stream const &) = delete;
        stream &operator=(stream const &) = delete;
        /// Movable
        stream(stream &&t) noexcept = default;
        stream &operator=(stream &&t) noexcept = default;
        ~stream() = default;

        FELSPAR_CORO_WRAPPER stream_awaitable<Y, handle_type> next();
    };


    template<typename Y, typename Allocator>
    struct stream_promise : private promise_allocator_impl<Allocator> {
        using promise_allocator_impl<Allocator>::operator new;
        using promise_allocator_impl<Allocator>::operator delete;

        std::coroutine_handle<> continuation = {};
        bool completed = false;
        memory::holding_pen<Y> value = {};
        std::exception_ptr eptr = {};

        using handle_type = unique_handle<stream_promise>;

        auto yield_value(Y y) {
            value.assign(std::move(y));
            return symmetric_continuation{std::exchange(continuation, {})};
        }

        void unhandled_exception() {
            eptr = std::current_exception();
            completed = true;
            value.reset();
        }
        void return_void() {
            completed = true;
            value.reset();
        }

        auto get_return_object() {
            return stream<Y, Allocator>{handle_type::from_promise(*this)};
        }

        auto initial_suspend() const noexcept { return std::suspend_always{}; }
        auto final_suspend() const noexcept {
            return symmetric_continuation{continuation};
        }
    };


    /// The awaitable type
    template<typename Y, typename H>
    class stream_awaitable {
        friend class cancellable;

      public:
        stream_awaitable(H &c) : continuation{c} {}
        ~stream_awaitable() { continuation.promise().continuation = {}; }

        bool await_ready() const noexcept {
            return continuation.promise().completed;
        }
        auto await_suspend(std::coroutine_handle<> awaiting) noexcept {
            continuation.promise().continuation = awaiting;
            return continuation.get();
        }
        memory::holding_pen<Y> await_resume() {
            if (auto eptr = continuation.promise().eptr) {
                std::rethrow_exception(eptr);
            } else {
                return std::move(continuation.promise().value).transfer_out();
            }
        }

      private:
        H &continuation;
    };
    template<typename Y, typename A>
    inline stream_awaitable<Y, typename stream<Y, A>::handle_type>
            stream<Y, A>::next() {
        return {yielding_coro};
    }


    /// Can be used to pipe streams into other streams
    template<typename Y, typename YA, typename X, typename XA>
    inline X operator|(stream<Y, YA> &&s, X (*c)(stream<Y, XA>)) {
        return c(std::move(s));
    }


}
