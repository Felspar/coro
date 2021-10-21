add_executable(primes-1-generator primes-1-generator.cpp)
target_link_libraries(primes-1-generator felspar-coro)

add_executable(primes-2-stream primes-2-stream.cpp)
target_link_libraries(primes-2-stream felspar-coro)

add_executable(primes-3-optimised primes-3-optimised.cpp)
target_link_libraries(primes-3-optimised felspar-coro)

add_executable(primes-4-allocator primes-4-allocator.cpp)
target_link_libraries(primes-4-allocator felspar-coro)
