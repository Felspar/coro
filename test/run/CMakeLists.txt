if(TARGET felspar-check)
    add_test_run(felspar-check felspar-coro TESTS
            generator.cpp
            lazy.cpp
            task.cpp
        )
endif()
