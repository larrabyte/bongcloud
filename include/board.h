#pragma once

#include <cstddef>

class Piece {
    public:
        enum Colour {
            white = 0b0,
            black = 0b1
        };

        enum Type {
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

        enum Colour colour;
        enum Type type;
};

class Board {
    public:
        // Board constructor.
        Board(std::size_t);

        // Load a FEN string onto the board.
        void loadfen(const char*);

        // Get the index of a given position.
        std::size_t square(const char *location);

        // Get the piece at a given index.
        Piece &square(std::size_t);

        // Get a pointer to the beginning of the board.
        Piece *begin(void);

        // Get a pointer to the end of the board (invalid address).
        Piece *end(void);

    private:
        std::size_t elements;
        std::size_t squares;
        Piece *array;
};
