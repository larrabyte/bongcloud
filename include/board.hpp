#pragma once

#include "pieces.hpp"

#include <centurion.hpp>
#include <cstddef>
#include <vector>

namespace bongcloud {
    // A square is just an optional piece.
    using square = std::optional<piece>;

    struct move {
        std::size_t from;
        std::size_t to;
    };

    struct capture {
        std::size_t index;
        bongcloud::piece piece;
    };

    struct mutation {
        bongcloud::move move;
        std::size_t trivials;
        std::optional<bongcloud::move> castle;
        std::optional<bongcloud::capture> capture;
        std::optional<bongcloud::piece> promotion;
    };

    class board {
        public:
            // The default board constructor, which takes a length and a boolean as parameters.
            // Setting the boolean to true disables all move checking.
            board(const std::size_t, const bool);

            // Returns a copy of the current board *without* history.
            board duplicate(void) const;

            // Prints out the current board state to stdout.
            void print(void) const;

            // Returns whether a player is currently in check.
            bool check(const piece::color) const;

            // Moves a piece from one square to another.
            // Returns true if the move was successful.
            bool mutate(const std::size_t, const std::size_t);

            // Overwrites the current board state using a FEN string.
            void load(const std::string_view);

            // Undoes the last move.
            void undo(void);

            // Returns a constant reference to the board's history array.
            const std::vector<mutation>& history(void) const noexcept {
                return m_history;
            }

            // Returns the last move made (may be std::nullopt).
            inline std::optional<move> latest(void) const noexcept {
                std::optional<move> m;

                if(m_history.size() > 0) {
                    m = m_history.back().move;
                }

                return m;
            }

            // Returns the color of the player whose turn it is to move.
            inline piece::color color(void) const noexcept {
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
            // Returns the type of a move based on piece movement rules.
            std::optional<piece::move> permissible(const std::size_t, const std::size_t) const;

            // The board's internal representation.
            std::vector<square> m_internal;

            // An array of previously made moves.
            std::vector<mutation> m_history;

            // Determines whether any move is legal.
            bool m_anarchy;

            // Whose turn it is to move.
            piece::color m_color = piece::color::white;

            // The number of trivial half-moves made.
            std::size_t m_trivial_half_moves = 0;
    };
}
