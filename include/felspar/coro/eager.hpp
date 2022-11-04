#pragma once


#include <felspar/coro/task.hpp>


namespace felspar::coro {


    /// Immediately launch a coroutine and then own it
    template<typename Task = task<void>>
    class eager {
      public:
        using task_type = Task;
        using promise_type = typename task_type::promise_type;
        using handle_type = typename promise_type::handle_type;

        /// Start a task immediately
        template<typename... PArgs, typename... MArgs>
        void post(task_type (*f)(PArgs...), MArgs &&...margs) {
            static_assert(sizeof...(PArgs) == sizeof...(MArgs));
            auto task = f(std::forward<MArgs>(margs)...);
            task.start();
            coro = task.release();
        }

        /// Return true if the task is already done
        bool done() const noexcept { return coro and coro.done(); }

      private:
        handle_type coro;
    };


}
