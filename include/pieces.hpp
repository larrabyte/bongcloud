#pragma once

#include "extras.hpp"

#include <concepts>
#include <cstddef>
#include <array>

namespace bcl {
    struct piece {
        // Defines every color of piece available.
        enum class color : unsigned char {
            white,
            black,
            first = white,
            last = black
        };

        // Defines every type of piece available.
        enum class type : unsigned char {
            pawn,
            knight,
            bishop,
            rook,
            queen,
            king,
            first = pawn,
            last = king
        };

        // Defines every type of move possible.
        enum class move : unsigned char {
            normal,
            capture,
            en_passant,
            short_castle,
            long_castle,
            promotion,
            first = normal,
            last = promotion
        };

        // The color of the piece.
        piece::color hue : 1;

        // The type of the piece.
        piece::type variety : 7;
    };

    namespace constants {
        // Defines every legal promotion piece.
        constexpr ext::array promotion_pieces = {
            piece::type::queen,
            piece::type::knight,
            piece::type::rook,
            piece::type::bishop
        };

        // Defines the values for each piece.
        constexpr ext::array piece_values = {
            1.0, // piece::type::pawn
            3.0, // piece::type::knight
            3.0, // piece::type::bishop
            5.0, // piece::type::rook
            9.0, // piece::type::queen
            0.0  // piece::type::king
        };

        // Names for each piece color.
        constexpr ext::array color_titles = {
            "white", // piece::color::white
            "black"  // piece::color::black
        };

        static_assert(
            piece_values.size() == ext::to_underlying(piece::type::last) + 1,
            "each piece type must have an associated value"
        );

        static_assert(
            color_titles.size() == ext::to_underlying(piece::color::last) + 1,
            "each piece color must have an associated name"
        );
    }

    // Allow for arithmetic operations on enumeration types.
    template<ext::enumerable A, std::integral B>
    A operator+(const A a, const B b) noexcept {
        return static_cast<A>(ext::to_underlying(a) + b);
    }

    template<ext::enumerable A, std::integral B>
    A operator-(const A a, const B b) noexcept {
        return static_cast<A>(ext::to_underlying(a) - b);
    }

    template<ext::enumerable A, std::integral B>
    A operator*(const A a, const B b) noexcept {
        return static_cast<A>(ext::to_underlying(a) * b);
    }

    template<ext::enumerable A, std::integral B>
    A operator/(const A a, const B b) noexcept {
        return static_cast<A>(ext::to_underlying(a) / b);
    }
}
