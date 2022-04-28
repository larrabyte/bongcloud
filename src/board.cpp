#include "board.hpp"

#include <centurion.hpp>
#include <fmt/core.h>
#include <cstddef>

bongcloud::board::board(const std::size_t l)
    : length {l},
      m_internal {std::vector<int>(l * l, 0)} {

    fmt::print("[bongcloud] initialising board of size {}x{}...\n", l, l);
}
