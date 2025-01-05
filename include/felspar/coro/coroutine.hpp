#pragma once


#include <utility>


#if __has_include(<coroutine>)
#include <coroutine>
namespace felspar::coro {
    template<typename T = void>
    using coroutine_handle = std::coroutine_handle<T>;
    using std::noop_coroutine;
    using suspend_always = std::suspend_always;
    using suspend_never = std::suspend_never;
}
#else
#include <experimental/coroutine>
namespace felspar::coro {
    template<typename T = void>
    using coroutine_handle = std::experimental::coroutine_handle<T>;
    using std::experimental::noop_coroutine;
    using suspend_always = std::experimental::suspend_always;
    using suspend_never = std::experimental::suspend_never;
}
#endif


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


    /// A type erased base type for the `unique_handle`
    struct unique_handle_base {
        virtual ~unique_handle_base() = default;
        virtual bool done() const noexcept = 0;
    };


    /// A wrapper around `coroutine_handle<P>` that manages the handle, calling
    /// `destroy` on it when done.
    template<typename P = void>
    class unique_handle final : public unique_handle_base {
        coroutine_handle<P> handle = {};
        explicit unique_handle(coroutine_handle<P> h) : handle{h} {}

      public:
        unique_handle() {}
        unique_handle(unique_handle const &) = delete;
        unique_handle(unique_handle &&h) noexcept
        : handle{std::exchange(h.handle, {})} {}
        ~unique_handle() override {
            if (handle) { handle.destroy(); }
        }

        unique_handle &operator=(unique_handle const &) = delete;
        unique_handle &operator=(unique_handle &&h) noexcept {
            if (handle) { handle.destroy(); }
            handle = std::exchange(h.handle, {});
            return *this;
        }

        /// Comparison
        template<typename T>
        bool operator==(unique_handle<T> c) const {
            return get() == c.get();
        }
        template<typename T>
        bool operator==(coroutine_handle<T> c) const {
            return get() == c;
        }

        /// Allow access to the underlying handle
        auto get() const noexcept { return handle; }
        auto release() noexcept { return std::exchange(handle, {}); }

        /// Forwarders for `coroutine_handle<P>` members
        explicit operator bool() const noexcept { return bool{handle}; }
        bool done() const noexcept override { return handle.done(); }
        template<typename PP>
        static auto from_promise(PP &&pp) noexcept {
            return unique_handle{
                    coroutine_handle<P>::from_promise(std::forward<PP>(pp))};
        }
        decltype(auto) promise() noexcept { return handle.promise(); }
        decltype(auto) promise() const noexcept { return handle.promise(); }
        void resume() noexcept { handle.resume(); }
    };


    /// Generally used from final_suspend when we need to execute a continuation
    /// after the coroutine has completed
    struct symmetric_continuation {
        coroutine_handle<> continuation;
        bool await_ready() const noexcept { return false; }
        coroutine_handle<> await_suspend(coroutine_handle<>) const noexcept {
            if (continuation) {
                return continuation;
            } else {
                return noop_coroutine();
            }
        }
        void await_resume() const noexcept {}
    };


}
