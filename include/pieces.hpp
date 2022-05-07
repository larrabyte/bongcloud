#pragma once

#include <cstddef>

namespace bongcloud {
    class piece {
        public:
            // Defines every colour of piece available.
            enum class colors {
                white,
                black
            };

            // Defines every type of piece available.
            enum class types {
                pawn,
                knight,
                bishop,
                rook,
                queen,
                king
            };

            // Defines every type of move possible.
            enum class moves {
                normal,
                capture,
                en_passant,
                short_castle,
                long_castle,
                promotion
            };

            // The piece constructor.
            piece(const colors c, const types t) : color(c), type(t) {}

            // The color of the piece.
            colors color;

            // The type of the piece.
            types type;

            // The number of times the piece has moved.
            std::size_t move_count = 0;
    };
}
