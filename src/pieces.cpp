#include "pieces.hpp"
#include "board.hpp"

#include <fmt/core.h>

std::size_t absolute_difference(const std::size_t a, const std::size_t b) noexcept {
    return (a > b) ? a - b : b - a;
}

bool bongcloud::board::is_movement_allowed(const std::size_t from, const std::size_t to) {
    // Assume that a piece is present at the origin square.
    const auto& origin = *m_internal[from].container;

    switch(origin.type) {
        case piece::type_t::pawn: {
            // Pawns can't move backwards or sideways.
            bool illegal = {
                (origin.color == piece::color_t::white && from > to) ||
                (origin.color == piece::color_t::black && from < to)
            };

            if(illegal) {
                return false;
            }

            // Pawns can move one square forward or two if they're on the first or second-last rank.
            // Since ranks are zero-indexed, this corresponds to the 1st and (length - 2)th ranks.
            // Pawns cannot take directly in-front of them.
            std::size_t difference = absolute_difference(from, to);
            bool one_forward = difference == length && !m_internal[to].container;

            bool two_forward = {
                difference == 2 * length &&
                (from / length == 1 || from / length == length - 2) &&
                !m_internal[to].container
            };

            // Pawns can diagonally capture if there is a piece present.
            bool diagonal_capture = {
                (difference == length - 1 || difference == length + 1) &&
                m_internal[to].container
            };

            return one_forward || two_forward || diagonal_capture;
        }

        case piece::type_t::knight: {
            return true;
        }

        case piece::type_t::bishop: {
            return true;
        }

        case piece::type_t::rook: {
            return true;
        }

        case piece::type_t::queen: {
            return true;
        }

        case piece::type_t::king: {
            return true;
        }

        // What piece is this??
        default:
            throw std::runtime_error("illegal piece type");
    }
}
