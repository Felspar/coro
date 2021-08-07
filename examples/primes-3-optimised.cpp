#include <felspar/coro/generator.hpp>
#include <felspar/coro/stream.hpp>
#include <felspar/coro/task.hpp>

#include <cstdint>
#include <iostream>
#include <vector>


using integer = std::uint64_t;


namespace {
    /**
     * Generate square numbers using only addition. This makes use of the fact
     * that the difference between squares is simply the odd numbers starting at
     * 3 (surprising but true).
     */
    felspar::coro::generator<std::pair<integer, integer>> square_numbers() {
        integer root{1}, square{1}, difference{1};
        while (true) {
            co_yield {root, square};
            ++root;
            square += (difference += 2);
        }
    }
    /**
     * The streams are identical to our implementations we've had before.
     */
    felspar::coro::stream<integer> numbers(integer upto) {
        for (integer number{2}; number <= upto; ++number) { co_yield number; }
    }
    felspar::coro::stream<integer>
            sieve(integer prime, felspar::coro::stream<integer> sieve) {
        for (auto checking = prime; auto value = co_await sieve.next();) {
            while (checking < *value) { checking += prime; }
            if (checking > *value) { co_yield *value; }
        }
    }


    felspar::coro::task<int> co_main() {
        integer found{};

        std::vector<integer> to_add;
        auto squares = square_numbers();
        integer root, square;
        std::tie(root, square) = *squares.next();

        /**
         * This implementation is fast enough to be able to generate primes all
         * the way up to my very own fragile prime (thanks Matt Parker)
         * https://youtu.be/p3Khnx0lUDE?t=1682
         */
        for (auto primes = numbers(694'183'367);
             auto prime = co_await primes.next();) {
            std::cout << *prime << ' ';
            ++found;
            /**
             * Every time we get to a prime which is larger than a square we
             * must add all of the primes we've found up to the root of that
             * prime to the sieve. This ensures that the sieve only ever
             * contains the minimum number of primes needed to test the current
             * number we're looking at.
             */
            to_add.push_back(*prime);
            /**
             * Whenever the prime we have just found is more than a perfect
             * square we may need to add to the sieve, but in any case we need
             * to know when the next square will come.
             */
            if (*prime > square) {
                std::tie(root, square) = *squares.next();
                /**
                 * Whenever we find a new perfect root and it is a prime we add
                 * it to the sieve
                 */
                if (to_add.front() == root) {
                    primes = sieve(root, std::move(primes));
                    to_add.erase(to_add.begin());
                }
            }
        }
        std::cout << "\nFound " << found << " primes\n";
        co_return 0;
    }
}


int main() { return co_main().get(); }
