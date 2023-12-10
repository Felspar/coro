cmake_minimum_required(VERSION 3.14...3.28)
project(felspar-coro LANGUAGES CXX)
enable_testing()


if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_custom_target(felspar-check)
    set_property(TARGET felspar-check PROPERTY EXCLUDE_FROM_ALL TRUE)
    add_custom_target(felspar-examples)
    include(requirements.cmake)
endif()


set(coro_opt)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
    set(coro_opt "$<$<COMPILE_LANGUAGE:CXX>:-fcoroutines-ts;-Wno-deprecated-experimental-coroutine>")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(coro_opt "$<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-experimental-coroutine>")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "NVHPC")
    set(coro_opt "$<$<COMPILE_LANGUAGE:CXX>:-fcoroutines>")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(coro_opt "$<$<COMPILE_LANGUAGE:CXX>:-fcoroutines;-Wno-mismatched-new-delete>")
endif()

add_library(felspar-coro INTERFACE)
target_include_directories(felspar-coro INTERFACE include)
target_compile_features(felspar-coro INTERFACE cxx_std_20)
target_compile_options(felspar-coro INTERFACE ${coro_opt})
target_link_libraries(felspar-coro INTERFACE felspar-memory)
install(DIRECTORY include/felspar TYPE INCLUDE)

if(TARGET felspar-examples)
    add_subdirectory(examples)
endif()

if(TARGET felspar-check)
    add_subdirectory(test)
endif()
