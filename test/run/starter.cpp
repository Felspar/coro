#include <felspar/coro/start.hpp>
#include <felspar/exceptions.hpp>
#include <felspar/test.hpp>


namespace {


    felspar::coro::task<void> co_throw(felspar::source_location loc) {
        throw felspar::stdexcept::runtime_error{"A test exception", loc};
        co_return;
    }
    felspar::coro::task<bool> co_true() { co_return true; }
    felspar::coro::task<bool> co_false() { co_return false; }
    felspar::coro::task<bool> co_throw_bool(felspar::source_location loc) {
        throw felspar::stdexcept::runtime_error{"A test exception", loc};
        co_return true;
    }


    auto const s = felspar::testsuite(
            "starter",
            [](auto check) {
                felspar::coro::starter<> s;
                s.post(co_throw, felspar::source_location::current());
                check(s.size()) == 1u;
                s.garbage_collect_completed();
                check(s.size()) == 0u;
            },
            [](auto check) {
                felspar::coro::starter<felspar::coro::task<bool>> s;
                s.post(co_true);
                check(s.size()) == 1u;
                s.garbage_collect_completed();
                check(s.size()) == 0u;
            },
            [](auto check) {
                felspar::coro::starter<> s;
                s.post(co_throw, felspar::source_location::current());
                check(s.size()) == 1u;
                check([&]() {
                    s.wait_for_all().get();
                }).throws(felspar::stdexcept::runtime_error{"A test exception"});
                check(s.size()) == 0u;
            },
            [](auto check) {
                felspar::coro::starter<felspar::coro::task<bool>> s;
                s.post(co_true);
                check(s.size()) == 1u;
                check(s.wait_for_all().get()) == 1u;
                check(s.size()) == 0u;
            },
            [](auto check) {
                felspar::coro::starter<felspar::coro::task<bool>> s;
                s.post(co_true);
                s.post(co_throw_bool, felspar::source_location::current());
                s.post(co_false);
                check(s.size()) == 3u;
                check(s.next().get()) == false;
                check([&]() {
                    s.next().get();
                }).throws(felspar::stdexcept::runtime_error{"A test exception"});
                check(s.next().get()) == true;
                check(s.size()) == 0u;
            });


}
