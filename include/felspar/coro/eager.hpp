#pragma once


#include <felspar/coro/task.hpp>


namespace felspar::coro {


    /// ## Immediately launch a single coroutine
    template<typename Task = task<void>>
    class eager {
      public:
        using task_type = Task;
        using promise_type = typename task_type::promise_type;
        using handle_type = typename promise_type::handle_type;


        /// ### Start a task immediately
        template<typename... PArgs, typename... MArgs>
        void post(task_type (*f)(PArgs...), MArgs &&...margs) {
            static_assert(sizeof...(PArgs) == sizeof...(MArgs));
            auto task = f(std::forward<MArgs>(margs)...);
            task.start();
            coro = task.release();
        }
        template<typename N, typename... PArgs, typename... MArgs>
        void post(N &o, task_type (N::*f)(PArgs...), MArgs &&...margs) {
            static_assert(sizeof...(PArgs) == sizeof...(MArgs));
            auto task = (o.*f)(std::forward<MArgs>(margs)...);
            task.start();
            coro = task.release();
        }


        /// ### Lifetime

        /// #### Return true if the task is already done
        bool done() const noexcept { return coro and coro.done(); }

        /// #### Release the held task so it can be `co_await`ed
        auto release() && { return task_type(std::move(coro)); }

        /// #### Destroy the contained coroutine
        auto destroy() { coro.release(); }


      private:
        handle_type coro;
    };


}
