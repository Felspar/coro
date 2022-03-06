#pragma once


#include <felspar/coro/coroutine.hpp>

#include <vector>


namespace felspar::coro {


    /**
     * Wait at the suspension point until resumed from an external location.
     */
    class cancellable {
        std::vector<coroutine_handle<>> continuations = {};
        bool signalled = false;

        void remove(coroutine_handle<> h) { std::erase(continuations, h); }

      public:
        cancellable() {}
        cancellable(cancellable const &) = delete;
        cancellable(cancellable &&) = delete;
        cancellable &operator=(cancellable const &) = delete;
        cancellable &operator=(cancellable &&) = delete;

        /// Used externally to cancel the controlled coroutine
        void cancel() {
            signalled = true;
            while (continuations.size()) {
                auto h = continuations.back();
                continuations.pop_back();
                h.resume();
            }
        }
        bool cancelled() const noexcept { return signalled; }

        /// Wrap an awaitable so that an early resumption can be signalled
        template<typename A>
        auto signal_or(A coro_awaitable) {
            struct awaitable {
                A a;
                cancellable &b;
                coroutine_handle<> continuation = {};

                ~awaitable() { b.remove(continuation); }

                bool await_ready() const noexcept {
                    return b.signalled or a.await_ready();
                }
                auto await_suspend(coroutine_handle<> h) noexcept {
                    /// `h` is the coroutine making use of the `cancellable`
                    continuation = h;
                    b.continuations.push_back(h);
                    return a.await_suspend(h);
                }
                auto await_resume()
                        -> decltype(std::declval<A>().await_resume()) {
                    b.remove(continuation);
                    if (b.signalled) {
                        a.continuation = {};
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
                coroutine_handle<> continuation = {};

                ~awaitable() { b.remove(continuation); }

                bool await_ready() const noexcept { return b.signalled; }
                void await_suspend(coroutine_handle<> h) noexcept {
                    continuation = h;
                    b.continuations.push_back(h);
                }
                void await_resume() noexcept { b.remove(continuation); }
            };
            return awaitable{*this};
        }
    };


}
