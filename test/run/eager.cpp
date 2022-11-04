#include <felspar/coro/eager.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("eager");


    felspar::coro::task<void> set_true(bool &s) {
        s = true;
        co_return;
    }
    auto const t = suite.test("task", [](auto check) {
        felspar::coro::eager<> t;
        bool started = false;
        t.post(set_true, std::ref(started));
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
    auto const o = suite.test("object", [](auto check) {
        felspar::coro::eager<> t;
        boolean object;
        t.post(object, &boolean::set_true);
        check(object.value) == true;
        check(t.done()) == true;
    });


}
