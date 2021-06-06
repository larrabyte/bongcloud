#pragma once

#include <cstddef>
#include "piece.h"

class Board {
    public:
        // Board constructor.
        Board(std::size_t);

        // Load a FEN string onto the board. Returns boolean depending on success.
        bool loadfen(const char*);

        // Move pieces and return a boolean depending on success.
        bool move(std::size_t, std::size_t);

        // Get the index of a given position.
        std::size_t square(const char*);

        // Get the index of a given position (file and rank, starting from 1).
        std::size_t square(std::size_t, std::size_t);

        // Get the piece at a given index.
        Piece& square(std::size_t);

        // Get a pointer to the beginning of the board.
        Piece* begin(void);

        // Get a pointer to the end of the board (invalid address).
        Piece* end(void);

    private:
        enum class Direction {
            north, northeast,
            east, southeast,
            south, southwest,
            west, northwest
        };

        // Return the index of the square at a given distance and direction.
        std::size_t square(Direction, std::size_t, std::size_t);

        // Return the rank of the index specified.
        std::size_t rank(std::size_t);

        // Return the file of the index specified.
        std::size_t file(std::size_t);

        // Advance the board state.
        void advance(Piece&, Piece&, std::size_t);

        std::size_t lastmove;
        Piece::Colour player;
        std::size_t elements;
        std::size_t stride;
        Piece* array;
};
