#pragma once


#include <coroutine>
#include <utility>


namespace felspar::coro {


    template<typename T = void>
    using coroutine_handle = std::coroutine_handle<T>;
    using std::noop_coroutine;
    using suspend_always = std::suspend_always;
    using suspend_never = std::suspend_never;


    /// A type erased base type for the `unique_handle`
    struct unique_handle_base {
        virtual ~unique_handle_base() = default;
        virtual bool done() const noexcept = 0;
    };


    /// A wrapper around `std::coroutine_handle<P>` that manages the handle,
    /// calling `destroy` on it when done.
    template<typename P = void>
    class unique_handle final : public unique_handle_base {
        std::coroutine_handle<P> handle = {};
        explicit unique_handle(std::coroutine_handle<P> h) : handle{h} {}

      public:
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

        /// Comparison
        template<typename T>
        bool operator==(unique_handle<T> c) const {
            return get() == c.get();
        }
        template<typename T>
        bool operator==(std::coroutine_handle<T> c) const {
            return get() == c;
        }

        /// Allow access to the underlying handle
        auto get() const noexcept { return handle; }
        auto release() noexcept { return std::exchange(handle, {}); }

        /// Forwarders for `std::coroutine_handle<P>` members
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


    /// Generally used from final_suspend when we need to execute a continuation
    /// after the coroutine has completed
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
