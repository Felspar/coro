include(FetchContent)

FetchContent_Declare(
        felspar-test
        GIT_REPOSITORY https://github.com/Felspar/test.git
        GIT_TAG main
    )
FetchContent_GetProperties(felspar-test)
if(NOT felspar-test_POPULATED)
    FetchContent_Populate(felspar-test)
    add_subdirectory(${felspar-test_SOURCE_DIR} ${felspar-test_BINARY_DIR})
endif()


FetchContent_Declare(
        felspar-exceptions
        GIT_REPOSITORY https://github.com/Felspar/exceptions.git
        GIT_TAG main
    )
FetchContent_GetProperties(felspar-exceptions)
if(NOT felspar-exceptions_POPULATED)
    FetchContent_Populate(felspar-exceptions)
    add_subdirectory(${felspar-exceptions_SOURCE_DIR} ${felspar-exceptions_BINARY_DIR})
endif()

FetchContent_Declare(
        felspar-memory
        GIT_REPOSITORY https://github.com/Felspar/memory.git
        GIT_TAG main
    )
FetchContent_GetProperties(felspar-memory)
if(NOT felspar-memory_POPULATED)
    FetchContent_Populate(felspar-memory)
    add_subdirectory(${felspar-memory_SOURCE_DIR} ${felspar-memory_BINARY_DIR})
endif()
