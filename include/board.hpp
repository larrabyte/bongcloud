#pragma once

#include <centurion.hpp>
#include <cstddef>
#include <vector>

namespace bongcloud {
    class board {
        public:
            // The board's constructor.
            board(const std::size_t);

            // Renders the board.
            void render(cen::renderer&);

            // To provide support for iteration.
            std::vector<int>::iterator begin(void) noexcept { return m_internal.begin(); }
            std::vector<int>::const_iterator begin(void) const noexcept { return m_internal.cbegin(); }
            std::vector<int>::iterator end(void) noexcept { return m_internal.end(); }
            std::vector<int>::const_iterator end(void) const noexcept { return m_internal.cend(); }

            // The length of the board.
            const std::size_t length;

        private:
            // The board's internal representation.
            std::vector<int> m_internal;
    };
}
