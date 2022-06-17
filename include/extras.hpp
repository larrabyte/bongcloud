#pragma once

#include <type_traits>
#include <concepts>
#include <cstddef>
#include <array>

namespace ext {
    template<typename T>
    concept enumerable = std::is_enum_v<T>;

    template<enumerable T>
    constexpr std::underlying_type_t<T> to_underlying(T t) noexcept {
        return static_cast<std::underlying_type_t<T>>(t);
    }

    template<enumerable T>
    constexpr T flip(T t) noexcept {
        auto v = to_underlying(t);
        return static_cast<T>(v ^ 0b1);
    }

    template<std::forward_iterator I, typename T>
    constexpr void replace_once(I first, I end, const T& stored, const T& replacement) {
        for(; first != end; ++first) {
            if(*first == stored) {
                *first = replacement;
                return;
            }
        }
    }

    template<typename T, std::size_t N>
    class array : public std::array<T, N> {
        public:
            template<std::integral I>
            const T& operator[](const I i) const noexcept {
                return std::array<T, N>::operator[](i);
            }

            template<std::integral I>
            T& operator[](const I i) noexcept {
                return std::array<T, N>::operator[](i);
            }

            template<enumerable I>
            const T& operator[](const I i) const noexcept {
                return std::array<T, N>::operator[](to_underlying(i));
            }

            template<enumerable I>
            T& operator[](const I i) noexcept {
                return std::array<T, N>::operator[](to_underlying(i));
            }
    };

    // Variadic deduction guide to allow for automated size and type deduction.
    template<typename T, typename... U>
    array(T, U...) -> array<T, 1 + sizeof...(U)>;
}
