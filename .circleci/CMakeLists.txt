project(coro-travis)
cmake_minimum_required(VERSION 3.10)

## Opt in to tests
add_custom_target(check)

add_subdirectory(.. coro)
