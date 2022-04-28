#include "pieces.hpp"
#include "board.hpp"

#include <centurion.hpp>
#include <fmt/core.h>
#include <cstddef>

// bongcloud::square constructors.
bongcloud::square::square(void) : piece(std::nullopt) {}
bongcloud::square::square(const bongcloud::piece& p) : piece(p) {}

bongcloud::board::board(const std::size_t l) : length {l}, m_internal {l * l, square()} {
    fmt::print("[bongcloud] initialising board of size {}x{}...\n", l, l);
}
