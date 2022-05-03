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
            std::size_t from_rank = from / length;
            std::size_t from_file = from % length;
            std::size_t to_rank = to / length;
            std::size_t to_file = to % length;

            // Knights can move in an L-shape: 2 units in one direction and 1 unit in another.
            std::size_t rank_difference = absolute_difference(from_rank, to_rank);
            std::size_t file_difference = absolute_difference(from_file, to_file);

            bool l_shape = {
                (rank_difference == 1 && file_difference == 2) ||
                (rank_difference == 2 && file_difference == 1)
            };

            return l_shape;
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
