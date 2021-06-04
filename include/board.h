#pragma once

#include <cstddef>
#include "piece.h"

class Board {
    public:
        // Board constructor.
        Board(std::size_t);

        // Load a FEN string onto the board. Returns boolean depending on success.
        bool loadfen(const char*);

        // Return a boolean depending on the legality of a move.
        bool islegal(std::size_t, std::size_t);

        // Return a boolean depending on whether the index is on the specified rank.
        bool onrank(const char, std::size_t);

        // Return a boolean depending on whether the index is on the specified file.
        bool onfile(const char, std::size_t);

        // Move to the next player.
        void advance(void);

        // Return the current player's colour.
        Piece::Colour current(void);

        // Get the index of a given position.
        std::size_t square(const char*);

        // Get the piece at a given index.
        Piece& square(std::size_t);

        // Get a pointer to the beginning of the board.
        Piece* begin(void);

        // Get a pointer to the end of the board (invalid address).
        Piece* end(void);

    private:
        Piece::Colour player;
        std::size_t elements;
        std::size_t squares;
        Piece* array;
};
