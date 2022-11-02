#pragma once


#include <felspar/coro/coroutine.hpp>

#include <optional>
#include <vector>


namespace felspar::coro {


    /// A data distribution bus
    /**
     * Data items can be fed into it and will be distributed to all of the
     * currently awaiting coroutines. The last value fed into it can also be
     * read at any time.
     *
     * The value type you used must be copyable. There is no thread
     * synchronisation, and waiting coroutines will be resumed in the same
     * thread that published the new value.
     */
    template<typename T>
    class bus {
        std::optional<T> current;
        std::vector<coroutine_handle<>> waiting, processing;

      public:
        bus() = default;

        /// Return an awaitable for the next value
        auto next() {
            struct awaitable {
                bus &b;
                bool await_ready() const noexcept { return false; }
                void await_suspend(felspar::coro::coroutine_handle<> h) {
                    b.waiting.push_back(h);
                }
                T const &await_resume() const { return *b.current; }
            };
            return awaitable{*this};
        }

        /// The latest value
        std::optional<T> const &latest() const { return current; }

        /// Publish this value to all awaiting. Returns the number of coroutines
        /// that received it.
        std::size_t push(T t) {
            current = std::move(t);
            std::swap(processing, waiting);
            std::size_t deliveries{};
            for (auto h : processing) {
                h.resume();
                ++deliveries;
            }
            processing.clear();
            return deliveries;
        }
    };


}
