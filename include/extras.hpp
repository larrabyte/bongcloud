#pragma once

#include <type_traits>

namespace ext {
    template<typename T>
    constexpr std::underlying_type_t<T> to_underlying(T t) noexcept {
        return static_cast<std::underlying_type_t<T>>(t);
    }
}
