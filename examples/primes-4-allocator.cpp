#include <felspar/coro/stream.hpp>
#include <felspar/coro/task.hpp>
#include <felspar/memory/slab.storage.hpp>

#include <cstdint>
#include <iostream>


using integer = std::uint64_t;


namespace {
    using allocator = felspar::memory::slab_storage<40 << 20>;

    felspar::coro::stream<integer, allocator>
            numbers(allocator &, integer upto) {
        for (integer number{2}; number <= upto; ++number) { co_yield number; }
    }
    felspar::coro::stream<integer, allocator>
            sieve(allocator &,
                  integer prime,
                  felspar::coro::stream<integer, allocator> sieve) {
        auto checking = prime;
        while (auto value = co_await sieve.next()) {
            while (checking < *value) { checking += prime; }
            if (checking > *value) { co_yield *value; }
        }
    }

    felspar::coro::task<int> co_main() {
        auto palloc = std::make_unique<allocator>();
        auto &alloc = *palloc;
        integer found{};
        auto primes = numbers(alloc, 1'000'000);
        while (auto prime = co_await primes.next()) {
            std::cout << *prime << ' ';
            ++found;
            primes = sieve(alloc, *prime, std::move(primes));
        }
        std::cout << "\nFound " << found << " primes\n";
        co_return 0;
    }
}


int main() { return co_main().get(); }
