/**
 * # Prime number generator using Sieve of Eratosthenes
 *
 * This method of finding primes has been around for a long time. It's named
 * after a [Greek mathematician from the third century
 * BCE](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes).
 *
 * Instead of testing numbers through division, the sieve produces primes using
 * only addition. This works by using connected coroutines each of which lets
 * through numbers that are not multiples of the prime it has been given.
 *
 * At the base of the chain is a generator that provides all of the numbers we
 * wish to test. This generator gives us our first prime, which is 2.
 *
 * We then place a generator in front of that which will check whether numbers
 * are divisible by 2 or not. This will tell us that 3 is our next prime. A new
 * generator placed at the front now checks for division by 3. We continue to
 * build this chain up one by one. When the current sieve chain produces a
 * number that number is a prime (because it's not divisible by any before it)
 * and we add a generator that sieves out multiples of that prime.
 */


#include <felspar/coro/generator.hpp>

#include <cstdint>
#include <iostream>


using integer = std::uint64_t;


namespace {
    /**
     * First of all we need all of the numbers that we want to check. We're
     * going to start at 2 because 1 isn't really a prime.
     */
    felspar::coro::generator<integer> numbers(integer upto) {
        for (integer number{2}; number <= upto; ++number) { co_yield number; }
    }
    /**
     * The sieve coroutine checks if the values that come out from lower down in
     * the sieve are multiples of the prime this part is going to check. Because
     * numbers coming in from the sieve before this keep increasing we need to
     * only remember the last number we checked against.
     */
    felspar::coro::generator<integer>
            sieve(integer prime, felspar::coro::generator<integer> sieve) {
        for (auto checking = prime; auto value = sieve.next();) {
            /**
             * If the number we're checking against is too low then keep adding
             * the prime we're checking until.
             */
            while (checking < *value) { checking += prime; }
            /**
             * We're now in one of two things situations:
             *
             * 1. The number is equal to the value, in which case the value is a
             * multiple of this prime and we must fetch the next value from the
             * sieve below us.
             * 2. The number is larger than the value we're checking which means
             * that the value is potentially prime as far as this check is
             * concerned.
             *
             * If we're in the second situation then this is certainly a prime
             * if this is the last part of the sieve, or potentially a prime if
             * this is lower down in the sieve, so we yield it to the next layer
             * up.
             */
            if (checking > *value) { co_yield *value; }
        }
    }
}


int main() {
    integer found{};
    for (auto primes = numbers(1'000'000); auto prime = primes.next();) {
        /**
         * All numbers that come out of the `primes` sieve are prime
         */
        std::cout << *prime << ' ';
        ++found;
        /**
         * We have to add that prime value to the `primes` sieve so that
         * multiples of it also get checked
         */
        primes = sieve(*prime, std::move(primes));
        /**
         * We actually only need to keep adding the prime to the  `primes` sieve
         * as long as the square of the prime is less than the top value we're
         * checking. This is a significant optimisation and is implemented in
         * [primes-3-optimised.cpp](./primes-3-optimised.cpp)
         */
    }
    std::cout << "\nFound " << found << " primes\n";
    return 0;
}
