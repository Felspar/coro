add_library(coro-headers-tests STATIC EXCLUDE_FROM_ALL
        allocator.cpp
        always.cpp
        bus.cpp
        cancellable.cpp
        eager.cpp
        future.cpp
        lazy.cpp
        start.cpp
        task.cpp
    )
target_link_libraries(coro-headers-tests felspar-coro)
add_dependencies(felspar-check coro-headers-tests)
