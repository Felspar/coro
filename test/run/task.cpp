#include <felspar/coro/task.hpp>
#include <felspar/test.hpp>

#include <memory>


namespace {


    auto const suite = felspar::testsuite("task");

    auto const bt = suite.test("basic task", [](auto check) {
        auto answer = []() -> felspar::coro::task<int> { co_return 42; };
        check(answer().get()) == 42;
    });

    auto const mv = suite.test("movable value", [](auto check) {
        struct X {
            std::unique_ptr<std::string> member;
            X(std::string s)
            : member{std::make_unique<std::string>(std::move(s))} {}
            bool operator==(X const &o) const { return *member == *o.member; }
        };
        auto marks_the_spot = []() -> felspar::coro::task<X> {
            co_return X{"spot"};
        };
        check(marks_the_spot().get()) == X{"spot"};
    });

    auto const iw = suite.test("ignored awaitable", [](auto check) {
        bool run = false;
        auto answer = [&]() -> felspar::coro::task<int> {
            run = true;
            co_return 42;
        };
        auto a = answer();
        check(run).is_falsey();
        check(std::move(a).get()) == 42;
        check(run).is_truthy();
    });

    auto const t = suite.test("throws", [](auto check) {
        bool run = false;
        auto throws = []() -> felspar::coro::task<int> {
            throw std::runtime_error{"Test throw"};
            co_return 42;
        };
        auto rt = throws();
        check(run).is_falsey();
        check([&]() {
            std::move(rt).get();
        }).throws(std::runtime_error{"Test throw"});
    });

    auto const nt = suite.test("nested task", [](auto check) {
        auto doubler = []() -> felspar::coro::task<int> {
            auto half_answer = []() -> felspar::coro::task<int> {
                co_return 21;
            };
            co_return 2 * co_await half_answer();
        };
        check(doubler().get()) == 42;
    });

    auto const ambr = suite.test("awaitable must be r-value", [](auto check) {
        auto half_answer = []() -> felspar::coro::task<int> { co_return 21; };
        auto doubler =
                [](felspar::coro::task<int> n) -> felspar::coro::task<int> {
            co_return 2 * co_await std::move(n);
        };
        check(doubler(half_answer()).get()) == 42;
    });

    auto const vt = suite.test("void task", [](auto check) {
        bool run = false;
        auto const empty = [](bool &r) -> felspar::coro::task<void> {
            r = true;
            co_return;
        };
        auto t = empty(run);
        check(run) == false;
        std::move(t).get();
        check(run) == true;
    });


}
