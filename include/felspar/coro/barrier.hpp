#pragma once

#include <felspar/coro/task.hpp>

#include <vector>


namespace felspar::coro {


    /// ## Barrier
    /**
     * The barrier allows for a number of coroutines to be held and then all
     * released at the same time.
     */
    template<typename Value, typename Task>
    class barrier final {
        std::optional<Value> value;
        std::vector<std::coroutine_handle<>> current{}, proc{};


      public:
        /// ### Construction
        barrier() = default;


        /// ### Queries
        std::size_t size() const noexcept { return current.size(); }
        std::optional<Value> const &last_value() const noexcept {
            return value;
        }


        /// ### Awaiting
        struct awaitable {
            barrier &owner;
            std::coroutine_handle<> mine = {};


            awaitable(barrier &b) : owner{b} {}
            awaitable(awaitable &&a)
            : owner{a.owner}, mine{std::exchange(a.mine, {})} {}
            ~awaitable() {
                if (mine) { std::erase(owner.current, mine); }
            }
            awaitable &operator=(awaitable &&);

            awaitable(awaitable const &) = delete;
            awaitable &operator=(awaitable const &) = delete;


            bool await_ready() const noexcept { return false; }
            void await_suspend(std::coroutine_handle<> h) {
                mine = h;
                owner.current.push_back(h);
            }
            Value const &await_resume() const noexcept { return *owner.value; }
        };
        awaitable operator co_await() { return {*this}; };


        /// ### Signalling
        void signal(Value v) {
            value = std::move(v);
            std::swap(current, proc);
            for (auto &c : proc) { c.resume(); }
            proc.clear();
        }
    };

    template<typename Task>
    class barrier<void, Task> final {
        std::vector<std::coroutine_handle<>> current{}, proc{};


      public:
        /// ### Construction
        barrier() = default;


        /// ### Queries
        std::size_t size() const noexcept { return current.size(); }


        /// ### Awaiting
        struct awaitable {
            barrier &owner;
            std::coroutine_handle<> mine = {};


            awaitable(barrier &b) : owner{b} {}
            awaitable(awaitable &&a)
            : owner{a.owner}, mine{std::exchange(a.mine, {})} {}
            ~awaitable() {
                if (mine) { std::erase(owner.current, mine); }
            }
            awaitable &operator=(awaitable &&);

            awaitable(awaitable const &) = delete;
            awaitable &operator=(awaitable const &) = delete;


            bool await_ready() const noexcept { return false; }
            void await_suspend(std::coroutine_handle<> h) {
                mine = h;
                owner.current.push_back(h);
            }
            void await_resume() const noexcept {}
        };
        awaitable operator co_await() { return {*this}; };


        /// ### Signalling
        void signal() {
            std::swap(current, proc);
            for (auto &c : proc) { c.resume(); }
            proc.clear();
        }
    };


}
