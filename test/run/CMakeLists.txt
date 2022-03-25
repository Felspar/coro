if(TARGET felspar-check)
    add_test_run(felspar-check felspar-coro TESTS
            generator.cpp
            lazy.cpp
            starter.cpp
            task.cpp
        )
endif()
