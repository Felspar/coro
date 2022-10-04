if(TARGET felspar-check)
    add_test_run(felspar-check felspar-coro TESTS
            generator.cpp
            lazy.cpp
            starter.cpp
            stream.cpp
            task.cpp
        )
endif()
