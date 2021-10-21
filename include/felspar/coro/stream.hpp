#pragma once


#include <felspar/coro/allocator.hpp>
#include <felspar/coro/coroutine.hpp>
#include <felspar/memory/holding_pen.hpp>


namespace felspar::coro {


    template<typename Y, typename H>
    class stream_awaitable;
    template<typename Y, typename Allocator = void>
    struct stream_promise;


    template<typename Y, typename Allocator = void>
    class stream final {
        friend struct stream_promise<Y, Allocator>;
        using handle_type = typename stream_promise<Y, Allocator>::handle_type;
        handle_type yielding_coro;

        stream(handle_type h) : yielding_coro{std::move(h)} {}

      public:
        using promise_type = stream_promise<Y, Allocator>;

        /// Not copyable
        stream(stream const &) = delete;
        stream &operator=(stream const &) = delete;
        /// Movable
        stream(stream &&t) noexcept = default;
        stream &operator=(stream &&t) noexcept = default;
        ~stream() = default;

        stream_awaitable<Y, handle_type> next();
    };


    template<typename Y, typename Allocator>
    struct stream_promise : private promise_allocator_impl<Allocator> {
        using promise_allocator_impl<Allocator>::operator new;
        using promise_allocator_impl<Allocator>::operator delete;

        coroutine_handle<> continuation = {};
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
        auto return_void() {
            completed = true;
            value.reset();
            return suspend_never{};
        }

        auto get_return_object() {
            return stream<Y, Allocator>{handle_type::from_promise(*this)};
        }

        auto initial_suspend() const noexcept { return suspend_always{}; }
        auto final_suspend() const noexcept {
            return symmetric_continuation{continuation};
        }
    };


    /// The awaitable type
    template<typename Y, typename H>
    class stream_awaitable {
      public:
        stream_awaitable(H &c) : yielding_coro{c} {}

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
                return std::move(yielding_coro.promise().value).transfer_out();
            }
        }

      private:
        H &yielding_coro;
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
