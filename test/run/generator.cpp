/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <felspar/coro/generator.hpp>
#include <felspar/test.hpp>

#include <vector>


namespace {


    felspar::coro::generator<bool> empty() { co_return; }

    felspar::coro::generator<std::size_t> fib() {
        std::size_t a{1}, b{1};
        co_yield 1u;
        while (true) { co_yield a = std::exchange(b, a + b); }
    }

    felspar::coro::generator<std::size_t> thrower(bool after_yield) {
        if (not after_yield)
            throw std::runtime_error{"Ooops, something went wrong"};
        co_yield 1;
        throw std::runtime_error{"Ooops, something went wrong after yield"};
    }

    template<typename T>
    felspar::coro::generator<T>
            take(std::size_t number, felspar::coro::generator<T> gen) {
        for (auto pos{gen.begin()}; number-- && pos != gen.end(); ++pos) {
            co_yield *pos;
        }
    }


    auto const g =
            felspar::testsuite("generator")
                    .test("empty",
                          [](auto check) {
                              auto e = empty();
                              check(e.begin()) == e.end();
                          })
                    .test("fibonacci",
                          [](auto check) {
                              auto f = fib();
                              auto pos = f.begin();
                              check(*pos) == 1u;
                              check(*++pos) == 1u;
                              check(*++pos) == 2u;
                              check(*++pos) == 3u;
                              check(*++pos) == 5u;
                              check(*++pos) == 8u;
                          })
                    .test("throws early",
                          [](auto check) {
                              auto f = thrower(false);
                              check([&]() { f.begin(); })
                                      .throws(std::runtime_error{
                                              "Ooops, something went wrong"});
                          })
                    .test("throws late",
                          [](auto check) {
                              auto f = thrower(true);
                              auto pos = f.begin();
                              check(*pos) == 1u;
                              check([&]() { ++pos; })
                                      .throws(std::runtime_error{
                                              "Ooops, something went wrong "
                                              "after "
                                              "yield"});
                          })
                    .test("terminates", [](auto check) {
                        std::vector<std::size_t> fibs{};
                        for (auto f : take(10, fib())) { fibs.push_back(f); }
                        check(fibs.size()) == 10u;
                        check(fibs.front()) == 1u;
                        check(fibs.back()) == 55u;
                    });


}
