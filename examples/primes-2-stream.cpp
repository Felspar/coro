#include <felspar/coro/stream.hpp>
#include <felspar/coro/task.hpp>

#include <cstdint>
#include <iostream>


using integer = std::uint64_t;


namespace {
    felspar::coro::stream<integer> numbers(integer upto) {
        for (integer number{2}; number <= upto; ++number) { co_yield number; }
    }
    felspar::coro::stream<integer>
            sieve(integer prime, felspar::coro::stream<integer> sieve) {
        auto checking = prime;
        while (auto value = co_await sieve.next()) {
            while (checking < *value) { checking += prime; }
            if (checking > *value) { co_yield *value; }
        }
    }

    felspar::coro::task<int> co_main() {
        integer found{};
        auto primes = numbers(1'000'000);
        while (auto prime = co_await primes.next()) {
            std::cout << *prime << ' ';
            ++found;
            primes = sieve(*prime, std::move(primes));
        }
        std::cout << "\nFound " << found << " primes\n";
        co_return 0;
    }
}


int main() { return co_main().get(); }
