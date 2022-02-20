project(coro-circleci)
cmake_minimum_required(VERSION 3.10)

## Opt in to tests
add_custom_target(felspar-check)
include(../requirements.cmake)

add_subdirectory(.. coro)
