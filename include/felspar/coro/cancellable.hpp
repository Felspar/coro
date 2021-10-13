#pragma once


#include <felspar/coro/coroutine.hpp>


namespace felspar::coro {


    /**
     * Wait at the suspension point until resumed from an external location.
     */
    class cancellable {
        coroutine_handle<> continuation = {};
        bool signalled = false;

        void resume_if_needed() {
            if (continuation) { std::exchange(continuation, {}).resume(); }
        }

      public:
        /// Used externally to cancel the controlled coroutine
        void cancel() {
            signalled = true;
            resume_if_needed();
        }
        bool cancelled() const noexcept { return signalled; }

        /// Wrap an awaitable so that an early resumption can be signalled
        template<typename A>
        auto signal_or(A coro_awaitable) {
            struct awaitable {
                A a;
                cancellable &b;

                bool await_ready() const noexcept {
                    return b.signalled or a.await_ready();
                }
                auto await_suspend(coroutine_handle<> h) noexcept {
                    /// `h` is the coroutine making use of the `cancellable`
                    b.continuation = h;
                    return a.await_suspend(h);
                }
                auto await_resume()
                        -> decltype(std::declval<A>().await_resume()) {
                    if (b.signalled) {
                        return {};
                    } else {
                        return a.await_resume();
                    }
                }
            };
            return awaitable{std::move(coro_awaitable), *this};
        }

        /// This can be directly awaited until signalled
        auto operator co_await() {
            struct awaitable {
                cancellable &b;

                bool await_ready() const noexcept { return b.signalled; }
                auto await_suspend(coroutine_handle<> h) noexcept {
                    b.continuation = h;
                }
                auto await_resume() noexcept {}
            };
            return awaitable{*this};
        }
    };


}
