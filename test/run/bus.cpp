#include <felspar/coro/bus.hpp>
#include <felspar/coro/start.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("bus");


    felspar::coro::task<void> copy_value(
            std::size_t &read, felspar::coro::bus<std::size_t> &values) {
        while (true) { read = co_await values.next(); }
    }


    auto const p = suite.test("process", [](auto check) {
        felspar::coro::bus<std::size_t> values;

        felspar::coro::starter<> proc;
        std::size_t read{};
        proc.post(copy_value, std::ref(read), std::ref(values));

        check(read) == 0u;
        check(values.latest().has_value()) == false;

        values.push(1);
        check(read) == 1u;
        check(values.latest().has_value()) == true;
        check(*values.latest()) == 1u;
    });


}
