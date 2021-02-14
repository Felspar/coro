#pragma once


#include <felspar/coro/generator.hpp>


namespace felspar::coro {


    /// A generator that always returns the same value
    template<typename V>
    inline generator<V> always(V v) {
        while (true) { co_yield v; }
    }


}
