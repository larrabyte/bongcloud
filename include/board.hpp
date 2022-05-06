#pragma once

#include "pieces.hpp"

#include <centurion.hpp>
#include <cstddef>
#include <vector>

namespace bongcloud {
    class square {
        public:
            // The default square constructor, which initialises the square to be empty.
            square(void) : piece(std::nullopt) {}

            // Another constructor, which takes a piece as an argument.
            square(const bongcloud::piece& p) : piece(p) {}

            // The piece that a square contains.
            std::optional<bongcloud::piece> piece;
    };

    class board {
        public:
            // The default board constructor, which takes a length and a boolean as parameters.
            // Setting the boolean to true disables all move checking.
            board(const std::size_t, const bool);

            // Prints out the current board state to stdout.
            void print(void) const;

            // Moves a piece from one square to another.
            // Returns true if the move was successful.
            bool move(const std::size_t, const std::size_t);

            // Overwrites the current board state using a FEN string.
            void load_fen(const std::string_view);

            // Returns the last move made (may be std::nullopt).
            inline std::optional<std::pair<std::size_t, std::size_t>> latest(void) const noexcept {
                return m_latest;
            }

            // Returns the color of the player whose turn it is to move.
            inline piece::colors color(void) const noexcept {
                return m_color;
            }

            // Provides support for range-based for loops.
            inline std::vector<square>::iterator begin(void) noexcept {
                return m_internal.begin();
            }

            inline std::vector<square>::const_iterator begin(void) const noexcept {
                return m_internal.cbegin();
            }

            inline std::vector<square>::iterator end(void) noexcept {
                return m_internal.end();
            }

            inline std::vector<square>::const_iterator end(void) const noexcept {
                return m_internal.cend();
            }

            // Allows the use of array indexing syntax to access board squares.
            inline square& operator[] (const std::size_t i) noexcept {
                return m_internal[i];
            }

            inline const square& operator[] (const std::size_t i) const noexcept {
                return m_internal[i];
            }

            // The length of the board.
            const std::size_t length;

        private:
            // Returns the permissibility of a move based on piece movement rules.
            bool permissible(const std::size_t, const std::size_t) const;

            // The board's internal representation.
            std::vector<square> m_internal;

            // The last move, represented as a (from, to) pair of indices.
            std::optional<std::pair<std::size_t, std::size_t>> m_latest;

            // Whose turn it is to move.
            piece::colors m_color = piece::colors::white;

            // Whether moves are checked before being played.
            bool m_anarchy;
    };
}
