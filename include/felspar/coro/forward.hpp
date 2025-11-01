#pragma once


namespace felspar::coro {


    template<typename Task>
    class eager;

    template<typename Task>
    class starter;

    template<typename Y, typename Allocator = void>
    class task;

    template<typename Y, typename Allocator>
    struct task_promise;


}
