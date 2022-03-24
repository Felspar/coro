#pragma once


#include <felspar/coro/task.hpp>

#include <vector>


namespace felspar::coro {


    template<typename Task = task<void>>
    class starter {
      public:
        using task_type = Task;
        using promise_type = typename task_type::promise_type;
        using handle_type = typename promise_type::handle_type;

        template<typename... PArgs, typename... MArgs>
        void post(task_type (*f)(PArgs...), MArgs &&...margs) {
            auto task = f(std::forward<MArgs>(margs)...);
            auto coro = task.release();
            coro.resume();
            live.push_back(std::move(coro));
        }

        /// Garbage collect old coroutines
        void gc() {
            /// TODO This implementation is not exception safe. If
            /// `consume_value` throws then the entire container is in a weird
            /// state
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

      private:
        std::vector<handle_type> live;
    };


}
