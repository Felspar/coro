#include <felspar/coro/eager.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("eager");


    felspar::coro::task<void> set_true(bool &s) {
        s = true;
        co_return;
    }
    auto const t = suite.test(
            "task",
            [](auto check) {
                felspar::coro::eager<> t;
                bool started = false;
                t.post(set_true, std::ref(started));
                check(started) == true;
                check(t.done()) == true;
            },
            [](auto check) {
                felspar::coro::eager<> t;
                bool started = false;
                t.post(set_true(started));
                check(started) == true;
                check(t.done()) == true;
            });


    struct boolean {
        bool value = false;

        felspar::coro::task<void> set_true() {
            value = true;
            co_return;
        }
    };
    auto const o = suite.test(
            "object",
            [](auto check) {
                felspar::coro::eager<> t;
                boolean object;
                t.post(object, &boolean::set_true);
                check(object.value) == true;
                check(t.done()) == true;
            },
            [](auto check) {
                felspar::coro::eager<> t;
                boolean object;
                t.post(object.set_true());
                check(object.value) == true;
                check(t.done()) == true;
            });


    felspar::coro::task<void> never_ends() {
        struct no_end {
            bool await_ready() const noexcept { return false; }
            void await_suspend(felspar::coro::coroutine_handle<>) noexcept {}
            void await_resume() {}
        };
        co_await no_end{};
    }
    auto const w = suite.test(
            "overwrite",
            [](auto check) {
                felspar::coro::eager<> t;
                bool started = false;
                t.post(set_true(started));
                check(started) == true;
                check(t.done()) == true;

                started = false;
                t.post(set_true(started));
                check(started) == true;
                check(t.done()) == true;
            },
            [](auto check) {
                felspar::coro::eager<> t;

                t.post(never_ends());
                check(t.done()) == false;

                t.post(never_ends());
                check(t.done()) == false;
            });


}
