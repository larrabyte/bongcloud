#pragma once

#include <type_traits>

namespace ext {
    template<typename T>
    constexpr std::underlying_type_t<T> to_underlying(T t) noexcept {
        return static_cast<std::underlying_type_t<T>>(t);
    }

    template<typename T>
    constexpr T flip(T t) noexcept {
        auto v = to_underlying(t);
        return static_cast<T>(v ^ 0b1);
    }

    template<typename I, typename T>
    constexpr void replace_once(I first, I end, const T& stored, const T& replacement) {
        for(; first != end; ++first) {
            if(*first == stored) {
                *first = replacement;
                return;
            }
        }
    }
}
