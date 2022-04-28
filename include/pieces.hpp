#pragma once

namespace bongcloud {
    class piece {
        public:
            // Defines every colour of piece available.
            enum class color_t {
                white,
                black
            };

            // Defines every type of piece available.
            enum class type_t {
                pawn,
                knight,
                bishop,
                rook,
                queen,
                king
            };

            // The piece constructor.
            piece(const color_t, const type_t);

            color_t color;
            type_t type;
    };
}
