#pragma once


#include <coroutine>
#include <utility>


#if defined __has_attribute
#if not defined FELSPAR_CORO_SKIP_LIFETIME_CHECKS \
        and __has_attribute(coro_return_type) \
        and __has_attribute(coro_lifetimebound) \
        and __has_attribute(coro_wrapper)
#define FELSPAR_CORO_CRT [[clang::coro_return_type, clang::coro_lifetimebound]]
#define FELSPAR_CORO_WRAPPER [[clang::coro_wrapper]]
#endif
#endif
#if not defined FELSPAR_CORO_CRT
#define FELSPAR_CORO_CRT
#endif
#if not defined FELSPAR_CORO_WRAPPER
#define FELSPAR_CORO_WRAPPER
#endif


namespace felspar::coro {


    /// ## Coroutine handles
    /**
     * A wrapper around `std::coroutine_handle<P>` that manages the handle,
     * calling `destroy` on it when done.
     */
    template<typename P = void>
    class unique_handle final {
        std::coroutine_handle<P> handle = {};
        explicit unique_handle(std::coroutine_handle<P> h) : handle{h} {}


      public:
        /// ### Construction and assignment
        unique_handle() {}
        unique_handle(unique_handle const &) = delete;
        unique_handle(unique_handle &&h) noexcept
        : handle{std::exchange(h.handle, {})} {}
        ~unique_handle() {
            if (handle) { handle.destroy(); }
        }

        unique_handle &operator=(unique_handle const &) = delete;
        unique_handle &operator=(unique_handle &&h) noexcept {
            if (handle) { handle.destroy(); }
            handle = std::exchange(h.handle, {});
            return *this;
        }


        /// ### Comparison
        template<typename T>
        bool operator==(unique_handle<T> c) const {
            return get() == c.get();
        }
        template<typename T>
        bool operator==(std::coroutine_handle<T> c) const {
            return get() == c;
        }


        /// ### Allow access to the underlying handle
        auto get() const noexcept { return handle; }
        auto release() noexcept { return std::exchange(handle, {}); }


        /// ### Forwarders for `coroutine_handle<P>` members
        explicit operator bool() const noexcept { return bool{handle}; }
        bool done() const noexcept { return handle.done(); }
        template<typename PP>
        static auto from_promise(PP &&pp) noexcept {
            return unique_handle{std::coroutine_handle<P>::from_promise(
                    std::forward<PP>(pp))};
        }
        decltype(auto) promise() noexcept { return handle.promise(); }
        decltype(auto) promise() const noexcept { return handle.promise(); }
        void resume() noexcept { handle.resume(); }
    };


    /// ## Symmetric continuation
    /**
     * Generally used from `final_suspend` when we need to execute a
     * continuation after the coroutine has completed.
     */
    struct symmetric_continuation {
        std::coroutine_handle<> continuation;
        bool await_ready() const noexcept { return false; }
        std::coroutine_handle<>
                await_suspend(std::coroutine_handle<>) const noexcept {
            if (continuation) {
                return continuation;
            } else {
                return std::noop_coroutine();
            }
        }
        void await_resume() const noexcept {}
    };


}
