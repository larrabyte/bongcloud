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

    struct record {
        piece::color color;
        bongcloud::move move;
        std::size_t trivials;
        std::optional<bongcloud::move> castle;
        std::optional<capture> capture;
        std::optional<piece> promotion;
    };

    class board {
        public:
            // User-defined constructor for setting initial board parameters.
            board(const std::size_t l, const bool a) noexcept : length {l}, m_internal {l * l}, m_anarchy {a} {};

            // Attempts to move a piece from one square to another.
            bool move(const std::size_t, const std::size_t);

            // Returns whether a player is currently in check.
            bool check(const piece::color) const;

            // Prints out the current board state to stdout.
            void print(void) const;

            // Overwrites the current board state using a FEN string.
            void load(const std::string_view);

            // Undoes the last move.
            void undo(void);

            // Returns a constant reference to the board's history array.
            const std::vector<record>& history(void) const noexcept {
                return m_history;
            }

            // Returns the last move made (may be std::nullopt).
            inline std::optional<bongcloud::move> latest(void) const noexcept {
                std::optional<bongcloud::move> m;

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
            // Returns the type of move (if pseudolegal) based on piece movement rules.
            std::optional<piece::move> pseudolegal(const std::size_t, const std::size_t) const;

            // The board's internal representation.
            std::vector<square> m_internal;

            // An array of previously made moves.
            std::vector<record> m_history;

            // Determines whether any move is legal.
            bool m_anarchy;

            // Whose turn it is to move.
            piece::color m_color = piece::color::white;

            // The number of trivial half-moves made.
            std::size_t m_trivials = 0;
    };
}
