#pragma once

#include <source_location>

namespace felspar::coro::detail {


    [[noreturn]] void throw_task_not_completed(std::source_location const &loc);
    [[noreturn]] void throw_task_not_completed_with_value(
            std::source_location const &loc);
    [[noreturn]] void throw_task_empty(std::source_location const &loc);
    [[noreturn]] void throw_future_no_value(std::source_location const &loc);
    [[noreturn]] void throw_future_already_set(std::source_location const &loc);
    [[noreturn]] void throw_starter_no_items(std::source_location const &loc);


}
