#pragma once

#include <cstddef>

namespace bongcloud {
    struct piece {
        // Defines every colour of piece available.
        enum class color : std::size_t {
            white,
            black,
            first = white,
            last = black
        };

        // Defines every type of piece available.
        enum class type : std::size_t {
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
        enum class move : std::size_t {
            normal,
            capture,
            en_passant,
            short_castle,
            long_castle,
            promotion,
            first = normal,
            last = promotion
        };

        // The piece constructor.
        piece(const piece::color c, const piece::type t) : hue(c), variety(t) {}

        // The color of the piece.
        piece::color hue;

        // The type of the piece.
        piece::type variety;

        // The number of times the piece has moved.
        std::size_t moves = 0;
    };
}
