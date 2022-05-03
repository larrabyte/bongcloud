#pragma once

#include "pieces.hpp"

#include <centurion.hpp>
#include <cstddef>
#include <vector>

namespace bongcloud {
    class square {
        public:
            // The default square constructor, which initialises the square to be empty.
            square(void);

            // Another constructors which takes a piece as an argument.
            square(const piece&);

            // The piece that a square contains.
            std::optional<piece> container;
    };

    class board {
        public:
            // The board's constructor.
            board(const std::size_t);

            // Move a piece from one square to another.
            void move(const std::size_t, const std::size_t);

            // Loads a FEN string into the board.
            void load_fen(const std::string_view);

            // Returns the last move.
            std::optional<std::pair<std::size_t, std::size_t>> last_move(void) const;

            // To provide support for iteration.
            std::vector<square>::iterator begin(void) noexcept { return m_internal.begin(); }
            std::vector<square>::const_iterator begin(void) const noexcept { return m_internal.cbegin(); }
            std::vector<square>::iterator end(void) noexcept { return m_internal.end(); }
            std::vector<square>::const_iterator end(void) const noexcept { return m_internal.cend(); }

            // To provide support for array indexing.
            square& operator[] (const std::size_t i) { return m_internal[i]; }
            const square& operator[] (const std::size_t i) const { return m_internal[i]; }

            // The length of the board.
            const std::size_t length;

        private:
            // The board's internal representation.
            std::vector<square> m_internal;

            // The last move made.
            std::optional<std::pair<std::size_t, std::size_t>> m_last_move;
    };
}
