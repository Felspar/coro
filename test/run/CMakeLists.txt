if(TARGET check)
    add_test_run(check felspar-coro TESTS
            generator.cpp
            lazy.cpp
            task.cpp
        )
endif()
