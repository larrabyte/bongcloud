#pragma once

namespace bongcloud {
    class piece {
        public:
            // Defines every colour of piece available.
            enum class color_t {
                white = 0b0,
                black = 0b1
            };

            // Defines every type of piece available.
            enum class type_t {
                pawn = 0b000,
                knight = 0b001,
                bishop = 0b010,
                rook = 0b011,
                queen = 0b100,
                king = 0b101
            };

            // The piece constructor.
            piece(const color_t, const type_t);

            color_t color;
            type_t type;
    };
}
