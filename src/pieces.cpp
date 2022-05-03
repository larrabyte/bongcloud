#include "pieces.hpp"
#include "board.hpp"

#include <fmt/core.h>

inline std::size_t absolute_difference(const std::size_t a, const std::size_t b) noexcept {
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
                (difference == 2 * length) &&
                (from / length == 1 || from / length == length - 2) &&
                (!m_internal[to].container)
            };

            // Pawns can diagonally capture if there is a piece present.
            bool diagonal_capture = {
                (difference == length - 1 || difference == length + 1) &&
                (m_internal[to].container)
            };

            // HON HON!
            bool en_passant = {
                (m_last_move) &&
                (m_internal[m_last_move->second].container->type == piece::type_t::pawn) &&
                (absolute_difference(m_last_move->first, m_last_move->second) / length == 2) &&
                ((origin.color == piece::color_t::white && to == m_last_move->second + length) ||
                (origin.color == piece::color_t::black && to == m_last_move->second - length))
            };

            return one_forward || two_forward || diagonal_capture || en_passant;
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
            std::size_t from_rank = from / length;
            std::size_t from_file = from % length;
            std::size_t to_rank = to / length;
            std::size_t to_file = to % length;

            // Bishops can move diagonally, therefore these differences must be equal.
            std::size_t rank_difference = absolute_difference(from_rank, to_rank);
            std::size_t file_difference = absolute_difference(from_file, to_file);
            if(rank_difference != file_difference) {
                return false;
            }

            // The minus one is so that the loop starts checking from the previous diagonal square.
            while(rank_difference - 1 > 0) {
                std::optional<std::size_t> index;

                if(from_rank < to_rank && from_file < to_file) {
                    index = from + (rank_difference - 1 * length) + file_difference;
                } else if(from_rank > to_rank && from_file < to_file) {
                    index = from - (rank_difference - 1 * length) + file_difference;
                } else if(from_rank > to_rank && from_file > to_file) {
                    index = from - (rank_difference - 1 * length) - file_difference;
                } else if(from_rank < to_rank && from_file > to_file) {
                    index = from + (rank_difference - 1 * length) - file_difference;
                }

                if(!index) {
                    // This should never occur.
                    throw std::runtime_error("illegal bishop legality check");
                }

                if(m_internal[*index].container) {
                    return false;
                }

                rank_difference--;
                file_difference--;
            }

            // If we're still here, then the previous loop didn't find any obstructing pieces.
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
