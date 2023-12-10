include(FetchContent)

FetchContent_Declare(
        felspar-test
        GIT_REPOSITORY https://github.com/Felspar/test.git
        GIT_TAG main
        GIT_SHALLOW true
    )
FetchContent_MakeAvailable(felspar-test)



FetchContent_Declare(
        felspar-exceptions
        GIT_REPOSITORY https://github.com/Felspar/exceptions.git
        GIT_TAG main
        GIT_SHALLOW true
    )
FetchContent_MakeAvailable(felspar-exceptions)


FetchContent_Declare(
        felspar-memory
        GIT_REPOSITORY https://github.com/Felspar/memory.git
        GIT_TAG main
        GIT_SHALLOW true
    )
FetchContent_MakeAvailable(felspar-memory)
