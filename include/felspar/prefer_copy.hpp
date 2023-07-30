#pragma once


#include <concepts>
#include <type_traits>


namespace felspar {


    template<typename T>
    concept prefer_copy = (std::copyable<T> and not std::movable<T>)
            or std::is_trivially_copy_constructible_v<T>;


}
