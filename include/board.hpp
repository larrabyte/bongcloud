#pragma once

#include <centurion.hpp>
#include <cstddef>
#include <vector>

namespace bongcloud {
    class board {
        public:
            // The board's constructor.
            board(std::size_t);

            // Renders the board.
            void render(cen::renderer&);

        private:
            // The length of the board.
            std::size_t m_length;

            // The board's internal representation.
            std::vector<int> m_internal;
    };
}
