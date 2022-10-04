#include <felspar/coro/task.hpp>
#include <felspar/coro/stream.hpp>
#include <felspar/test.hpp>

#include <variant>


namespace {


    auto const suite = felspar::testsuite("stream");


    felspar::coro::stream<int> numbers(int upto) {
        for (int n{}; n < upto; ++n) { co_yield n; }
    }
    felspar::coro::stream<std::string> strings() {
        co_yield "one";
        co_yield "two";
        co_yield "three";
    }
    felspar::coro::stream<std::variant<std::string, int>> interleaved(
            felspar::coro::stream<std::string> strings,
            felspar::coro::stream<int> numbers) {
        while (true) {
            auto s = co_await strings.next();
            if (not s) { co_return; }
            co_yield *s;

            auto n = co_await numbers.next();
            if (not n) { co_return; }
            co_yield *n;
        }
    }


    auto const sint = suite.test("int", [](auto check) {
        [&]() -> felspar::coro::task<void> {
            int expected{};
            for (auto nums = numbers(5); auto n = co_await nums.next();) {
                check(*n) == expected++;
            }
        }()
                         .get();
    });


    auto const sstring = suite.test("string", [](auto check) {
        [&]() -> felspar::coro::task<void> {
            auto str = strings();
            auto one = co_await str.next();
            check(one.value()) == "one";
            auto two = co_await str.next();
            check(two.value()) == "two";
            auto three = co_await str.next();
            check(three.value()) == "three";
        }()
                         .get();
    });


    auto const svariant = suite.test("variant", [](auto check) {
        [&]() -> felspar::coro::task<void> {
            auto vars = interleaved(strings(), numbers(5));

            auto onestr = co_await vars.next();
            check(std::get<std::string>(onestr.value())) == "one";
            auto onen = co_await vars.next();
            check(std::get<int>(onen.value())) == 0;

            auto twostr = co_await vars.next();
            check(std::get<std::string>(twostr.value())) == "two";
            auto twon = co_await vars.next();
            check(std::get<int>(twon.value())) == 1;

            auto threestr = co_await vars.next();
            check(std::get<std::string>(threestr.value())) == "three";
            auto threen = co_await vars.next();
            check(std::get<int>(threen.value())) == 2;

            auto done = co_await vars.next();
            check(done).is_falsey();
        }()
                         .get();
    });


}
