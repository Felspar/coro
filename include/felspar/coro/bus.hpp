#pragma once


#include <felspar/coro/coroutine.hpp>
#include <felspar/coro/task.hpp>

#include <optional>
#include <vector>


namespace felspar::coro {


    /// ## A data distribution bus
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
    class bus final {
        std::optional<T> current;
        std::vector<coroutine_handle<>> waiting, processing;

      public:
        bus() = default;


        /// ### Query the bus

        /// #### Clients
        /// Returns true if there is anything currently waiting on a bus value,
        /// or if value processing is ongoing
        bool has_clients() const noexcept {
            return not waiting.empty() or not processing.empty();
        }

        /// #### The latest value
        /// This will only be empty until the first value is pushed
        std::optional<T> const &latest() const { return current; }


        /// ### Return an awaitable for the next value
        auto next() {
            struct awaitable {
                bus &b;
                coroutine_handle<> waiting_handle = {};
                ~awaitable() {
                    if (waiting_handle) {
                        std::erase(b.waiting, waiting_handle);
                        std::erase(b.processing, waiting_handle);
                    }
                }

                bool await_ready() const noexcept { return false; }
                void await_suspend(felspar::coro::coroutine_handle<> h) {
                    waiting_handle = h;
                    b.waiting.push_back(h);
                }
                T const &await_resume() {
                    waiting_handle = {};
                    return *b.current;
                }
            };
            return awaitable{*this};
        }


        /// ### Publish a value to all waiting coroutines
        /**
         * Returns the number of coroutines that received a copy of the message.
         *
         * Waiting coroutines are continued synchronously. This can lead to
         * undefined behaviour if it is a coroutine that has pushed the value
         * and the value causes the coroutine's stack frame to be destroyed.
         */
        std::size_t push(T t) {
            current = std::move(t);
            std::swap(processing, waiting);
            std::size_t const deliveries{processing.size()};
            while (not processing.empty()) {
                auto handle = processing.back();
                processing.pop_back();
                handle.resume();
            }
            return deliveries;
        }


        /// ### Forward values to another bus
        template<typename F = T>
        felspar::coro::task<void> forward(bus<F> &b) {
            while (true) { b.push(co_await next()); }
        }
        template<typename F = T, typename Allocator>
        felspar::coro::task<void, Allocator> forward(Allocator &, bus<F> &b) {
            while (true) { b.push(co_await next()); }
        }
    };


}
