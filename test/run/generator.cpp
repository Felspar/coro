#include <felspar/coro/generator.hpp>
#include <felspar/memory/stack.storage.hpp>
#include <felspar/test.hpp>

#include <vector>


namespace {


    felspar::coro::generator<bool> empty() { co_return; }

    felspar::coro::generator<std::size_t> fib() {
        std::size_t a{1}, b{1};
        co_yield 1u;
        while (true) {
            a = std::exchange(b, a + b);
            co_yield a;
        }
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

    struct move_only {
        std::string text = "string";

        move_only() {}
        move_only(move_only const &) = delete;
        move_only(move_only &&) = default;

        move_only operator=(move_only const &) = delete;
        move_only operator=(move_only &&) = delete;
    };
    felspar::coro::generator<move_only> mover() { co_yield move_only{}; }


    auto const gi =
            felspar::testsuite("generator/iteration")
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
                    .test("terminates",
                          [](auto check) {
                              std::vector<std::size_t> fibs{};
                              for (auto f : take(10, fib())) {
                                  fibs.push_back(f);
                              }
                              check(fibs.size()) == 10u;
                              check(fibs.front()) == 1u;
                              check(fibs.back()) == 55u;
                          })
                    .test("move_only", [](auto check) {
                        auto moves = mover();
                        for (auto &&mo : moves) { check(mo.text) == "string"; }
                    });


    auto const gn =
            felspar::testsuite("generator/next")
                    .test("empty",
                          [](auto check) {
                              auto e = empty();
                              check(e.next()).is_falsey();
                          })
                    .test("fibonacci",
                          [](auto check) {
                              auto f = fib();
                              check(f.next()) == 1u;
                              check(f.next()) == 1u;
                              check(f.next()) == 2u;
                              check(f.next()) == 3u;
                              check(f.next()) == 5u;
                              check(f.next()) == 8u;
                          })
                    .test("throws early",
                          [](auto check) {
                              auto f = thrower(false);
                              check([&]() { f.next(); })
                                      .throws(std::runtime_error{
                                              "Ooops, something went wrong"});
                          })
                    .test("throws late",
                          [](auto check) {
                              auto f = thrower(true);
                              check(f.next()) == 1u;
                              check([&]() { f.next(); })
                                      .throws(std::runtime_error{
                                              "Ooops, something went wrong "
                                              "after "
                                              "yield"});
                          })
                    .test("terminates",
                          [](auto check) {
                              auto t = take(10, fib());
                              check(t.next()) == 1u;
                              check(t.next()) == 1u;
                              check(t.next()) == 2u;
                              check(t.next()) == 3u;
                              check(t.next()) == 5u;
                              check(t.next()) == 8u;
                              check(t.next()) == 13u;
                              check(t.next()) == 21u;
                              check(t.next()) == 34u;
                              check(t.next()) == 55u;
                              check(t.next()).is_falsey();
                          })
                    .test("move_only", [](auto check) {
                        auto m = mover();
                        check(m.next()->text) == "string";
                        check(m.next()).is_falsey();
                    });


#ifndef NDEBUG
    felspar::coro::generator<std::size_t, felspar::memory::stack_storage<>>
            alloc_fib(felspar::memory::stack_storage<> &) {
        std::size_t a{1}, b{1};
        co_yield 1u;
        while (true) {
            a = std::exchange(b, a + b);
            co_yield a;
        }
    }


    auto const al = felspar::testsuite("generator/allocator")
                            .test("alloc", [](auto check) {
                                felspar::memory::stack_storage alloc;
                                auto const start_free = alloc.free();

                                auto fib = alloc_fib(alloc);
                                check(alloc.free()) < start_free;
                            });
#endif


}
