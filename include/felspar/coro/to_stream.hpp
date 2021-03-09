#pragma once


#include <felspar/coro/generator.hpp>
#include <felspar/coro/stream.hpp>


namespace felspar::coro {


    /// Convert a `generator<V>` to a `stream<V>`
    template<typename V>
    inline stream<V> to_stream(generator<V> g) {
        for (auto &&v : g) { co_yield std::move(v); }
    }


}
