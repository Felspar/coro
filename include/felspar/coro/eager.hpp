#pragma once


#include <felspar/coro/task.hpp>


namespace felspar::coro {


    /// ## Immediately launch a single coroutine
    template<typename Task = task<void>>
    class eager {
      public:
        using task_type = Task;
        using promise_type = typename task_type::promise_type;
        using unique_handle_type = typename promise_type::unique_handle_type;


        /// ### Start a task immediately
        eager() {}
        explicit eager(task_type t) { post(std::move(t)); }
        template<typename... PArgs, typename... MArgs>
        void post(task_type (*f)(PArgs...), MArgs &&...margs) {
            static_assert(sizeof...(PArgs) == sizeof...(MArgs));
            post(f(std::forward<MArgs>(margs)...));
        }
        template<typename N, typename... PArgs, typename... MArgs>
        void post(N &o, task_type (N::*f)(PArgs...), MArgs &&...margs) {
            static_assert(sizeof...(PArgs) == sizeof...(MArgs));
            post((o.*f)(std::forward<MArgs>(margs)...));
        }
        void post(task_type t) {
            t.start();
            coro = t.release();
        }


        /// ### Lifetime

        /// #### Return true if the task is already done
        bool done() const noexcept { return coro and coro.done(); }

        /// #### Release the held task so it can be `co_await`ed
        FELSPAR_CORO_WRAPPER auto release() && {
            return task_type(std::move(coro));
        }

        /// #### Destroy the contained coroutine
        auto destroy() {
            if (auto h = coro.release(); h) { h.destroy(); }
        }


      private:
        unique_handle_type coro;
    };


}
