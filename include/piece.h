#pragma once

class Piece {
    public:
        enum class Colour {
            white = 0b0,
            black = 0b1
        };

        enum class Type {
            empty = 0b000,
            pawn = 0b001,
            knight = 0b010,
            bishop = 0b011,
            rook = 0b100,
            queen = 0b101,
            king = 0b110
        };

        // Set a piece to a certain colour/type.
        void set(Colour, Type);

        Colour colour;
        Type type;
};
