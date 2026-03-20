#include <felspar/coro/exception.details.hpp>
#include <felspar/exceptions/runtime_error.hpp>
#include <felspar/exceptions/logic_error.hpp>

#include <stdexcept>


void felspar::coro::detail::throw_task_not_completed(
        std::source_location const &loc) {
    throw felspar::stdexcept::runtime_error{"The task hasn't completed", loc};
}


void felspar::coro::detail::throw_task_not_completed_with_value(
        std::source_location const &loc) {
    throw felspar::stdexcept::runtime_error{
            "The task hasn't completed with a value ", loc};
}


void felspar::coro::detail::throw_task_empty(std::source_location const &loc) {
    throw felspar::stdexcept::runtime_error{"Cannot start an empty task", loc};
}


void felspar::coro::detail::throw_future_no_value(
        std::source_location const &loc) {
    throw felspar::stdexcept::logic_error{
            "Future does not contain a value", loc};
}


void felspar::coro::detail::throw_future_already_set(
        std::source_location const &loc) {
    throw felspar::stdexcept::logic_error{
            "The future already has a value set", loc};
}


void felspar::coro::detail::throw_starter_no_items(
        std::source_location const &loc) {
    throw felspar::stdexcept::logic_error{
            "Cannot call starter::next() if there are no items", loc};
}
