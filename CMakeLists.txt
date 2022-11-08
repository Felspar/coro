cmake_minimum_required(VERSION 3.10)
project(felspar-coro)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_custom_target(felspar-check)
    set_property(TARGET felspar-check PROPERTY EXCLUDE_FROM_ALL TRUE)
    add_custom_target(felspar-examples)
    include(requirements.cmake)
endif()

add_library(felspar-coro INTERFACE)
target_include_directories(felspar-coro INTERFACE include)
target_compile_features(felspar-coro INTERFACE cxx_std_20)
target_link_libraries(felspar-coro INTERFACE felspar-memory)
install(DIRECTORY include/felspar DESTINATION include)

if(TARGET felspar-examples)
    add_subdirectory(examples)
endif()

if(TARGET felspar-check)
    add_subdirectory(test)
endif()
