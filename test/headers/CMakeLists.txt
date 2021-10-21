add_library(coro-headers-tests STATIC EXCLUDE_FROM_ALL
        allocator.cpp
        always.cpp
        cancellable.cpp
        lazy.cpp
        task.cpp
    )
target_link_libraries(coro-headers-tests felspar-coro)
add_dependencies(check coro-headers-tests)
