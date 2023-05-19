#pragma once


#include <felspar/coro/task.hpp>
#include <felspar/exceptions.hpp>

#include <vector>


namespace felspar::coro {


    /// ## Start and manage multiple coroutines
    template<typename Task = task<void>>
    class starter {
      public:
        using task_type = Task;
        using promise_type = typename task_type::promise_type;
        using handle_type = typename promise_type::handle_type;

        /// ### Start a new coroutine
        template<typename... PArgs, typename... MArgs>
        void post(task_type (*f)(PArgs...), MArgs &&...margs) {
            static_assert(sizeof...(PArgs) == sizeof...(MArgs));
            auto task = f(std::forward<MArgs>(margs)...);
            task.start();
            auto coro = task.release();
            live.push_back(std::move(coro));
        }
        template<typename N, typename... PArgs, typename... MArgs>
        void post(N &o, task_type (N::*f)(PArgs...), MArgs &&...margs) {
            static_assert(sizeof...(PArgs) == sizeof...(MArgs));
            auto task = (o.*f)(std::forward<MArgs>(margs)...);
            task.start();
            auto coro = task.release();
            live.push_back(std::move(coro));
        }

        /// ### The number of coroutines currently held by the starter
        [[nodiscard]] std::size_t size() const noexcept { return live.size(); }
        [[nodiscard]] bool empty() const noexcept { return live.empty(); }

        /// ### Garbage collect old coroutines
        /// Ignores any errors and return values.
        void garbage_collect_completed() {
            live.erase(
                    std::remove_if(
                            live.begin(), live.end(),
                            [](auto const &h) { return h.done(); }),
                    live.end());
        }
        [[deprecated(
                "Use the new garbage_collect_completed or wait_for_all "
                "APIs")]] void
                gc() {
            live.erase(
                    std::remove_if(
                            live.begin(), live.end(),
                            [](auto const &h) {
                                if (h.done()) {
                                    h.promise().consume_value();
                                    return true;
                                } else {
                                    return false;
                                }
                            }),
                    live.end());
        }

        /// ### Wait for all coroutines to complete
        /**
         * Or for the first to throw an exception. If no exception has happened
         * then returns the number of coroutines awaited.
         */
        task<std::size_t> wait_for_all() {
            std::size_t count{};
            while (live.size()) {
                task_type t{std::move(live.back())};
                live.pop_back();
                co_await std::move(t);
                ++count;
            }
            co_return count;
        }

        /// ### The next item in line in the starter
        task_type next(source_location const &loc = source_location::current()) {
            if (live.empty()) {
                throw stdexcept::logic_error{
                        "Cannot call starter::next() if there are no items",
                        loc};
            } else {
                task_type t{std::move(live.back())};
                live.pop_back();
                return t;
            }
        }

        /// ### Delete all coroutines
        /**
         * Calling this from any of the running coroutines will invoke undefined
         * behaviour.
         */
        void reset() { live.clear(); }

      private:
        std::vector<handle_type> live;
    };


}
