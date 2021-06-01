#pragma once

#include <cstddef>
#include "piece.h"

class Board {
    public:
        // Board constructor.
        Board(std::size_t);

        // Load a FEN string onto the board. Returns boolean depending on success.
        bool loadfen(const char*);

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
