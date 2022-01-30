#include <felspar/coro/lazy.hpp>
#include <felspar/memory/slab.storage.hpp>
#include <felspar/test.hpp>


namespace {


    auto const lazy =
            felspar::testsuite("lazy")
                    .test("unevaluated",
                          [](auto check) {
                              bool run = false;
                              auto const notrun =
                                      [&]() -> felspar::coro::lazy<int> {
                                  run = true;
                                  co_return 0;
                              };
                              auto c = notrun();
                              check(run).is_falsey();
                          })
                    .test("evaluated_by_get",
                          [](auto check) {
                              std::size_t runs = 0;
                              auto const notrun =
                                      [&]() -> felspar::coro::lazy<int> {
                                  ++runs;
                                  co_return 42;
                              };
                              auto c = notrun();
                              check(runs) == 0u;
                              check(c()) == 42;
                              check(runs) == 1u;
                              check(c()) == 42;
                              check(runs) == 1u;
                              check(c()) == 42;
                              check(runs) == 1u;
                          })
                    .test("throws", [](auto check) {
                        auto const thrower = []() -> felspar::coro::lazy<int> {
                            throw std::runtime_error{"Test exception"};
                        };
                        check([&]() {
                            thrower()();
                        }).throws(std::runtime_error{"Test exception"});
                    });


#ifndef NDEBUG
    felspar::coro::lazy<int, felspar::memory::slab_storage<>>
            track_run(felspar::memory::slab_storage<> &, bool &r) {
        r = true;
        co_return 1234;
    }
    auto const af = lazy.test("allocator/function", [](auto check) {
        felspar::memory::slab_storage<> slab;
        auto const start_free = slab.free();
        auto run = false;

        auto lazy = track_run(slab, run);
        check(slab.free()) < start_free;
        check(run) == false;

        check(lazy()) == 1234;
        check(run) == true;
    });
    auto const am = lazy.test("allocator/member", [](auto check) {
        felspar::memory::slab_storage<> slab;
        auto const start_free = slab.free();
        auto run = false;

        auto const coro = [](auto &, bool &r)
                -> felspar::coro::lazy<int, felspar::memory::slab_storage<>> {
            r = true;
            co_return 42;
        };

        auto lazy = coro(slab, run);
        check(slab.free()) < start_free;
        check(run) == false;

        check(lazy()) == 42;
        check(run) == true;
    });
#endif


}
