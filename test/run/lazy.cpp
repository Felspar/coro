#include <felspar/coro/lazy.hpp>
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


}
