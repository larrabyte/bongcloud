#pragma once

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

            // The piece constructor.
            piece(const colors c, const types t) : color(c), type(t) {}

            colors color;
            types type;
    };
}
