#pragma once


#include <felspar/coro/coroutine.hpp>

#include <vector>


namespace felspar::coro {


    /// ## Cancellable coroutines
    class cancellable {
        std::vector<std::coroutine_handle<>> continuations = {};
        bool signalled = false;

        void remove(std::coroutine_handle<> h) { std::erase(continuations, h); }

      public:
        cancellable() {}
        cancellable(cancellable const &) = delete;
        cancellable(cancellable &&) = delete;
        cancellable &operator=(cancellable const &) = delete;
        cancellable &operator=(cancellable &&) = delete;


        /// ### `cancel`
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


        /// ### `signal_or`
        /// Wrap an awaitable so that an early resumption can be signalled
        template<typename A>
        FELSPAR_CORO_WRAPPER auto signal_or(A coro_awaitable) {
            struct FELSPAR_CORO_CRT awaitable {
                A a;
                cancellable &b;
                std::coroutine_handle<> continuation = {};

                ~awaitable() { b.remove(continuation); }

                bool await_ready() const noexcept {
                    return b.signalled or a.await_ready();
                }
                auto await_suspend(std::coroutine_handle<> h) noexcept {
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

        /// ### `operator co_await`
        /// This type can also be directly awaited until signalled
        FELSPAR_CORO_WRAPPER auto operator co_await() {
            struct FELSPAR_CORO_CRT awaitable {
                cancellable &b;
                std::coroutine_handle<> continuation = {};

                ~awaitable() { b.remove(continuation); }

                bool await_ready() const noexcept { return b.signalled; }
                void await_suspend(std::coroutine_handle<> h) noexcept {
                    continuation = h;
                    b.continuations.push_back(h);
                }
                void await_resume() noexcept { b.remove(continuation); }
            };
            return awaitable{*this};
        }
    };


}
