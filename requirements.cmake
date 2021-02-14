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
