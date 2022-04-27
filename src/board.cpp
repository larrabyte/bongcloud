#include "board.hpp"

#include <centurion.hpp>
#include <fmt/core.h>
#include <cstddef>

bongcloud::board::board(std::size_t length) {
    fmt::print("[bongcloud] initialising board of size {}x{}...\n", length, length);
    this->length = length;
    this->internal = std::vector<int>(length * length, 0);
}
