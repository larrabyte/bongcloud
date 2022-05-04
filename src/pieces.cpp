#include "pieces.hpp"
#include "board.hpp"

#include <fmt/core.h>

namespace internal {
    inline std::size_t absdiff(const std::size_t a, const std::size_t b) noexcept {
        return (a > b) ? a - b : b - a;
    }

    namespace obstructions {
        bool bishop(
            const bongcloud::board& surface,
            const std::size_t origin,
            const std::size_t from_rank,
            const std::size_t from_file,
            const std::size_t to_rank,
            const std::size_t to_file,
            std::size_t rank_difference,
            std::size_t file_difference) {

            while(rank_difference > 0 && file_difference > 0) {
                std::size_t index;

                // The destination square is to the top-right of the origin square.
                if(from_rank < to_rank && from_file < to_file) {
                    index = origin + (rank_difference * surface.length) + file_difference;
                }

                // The destination square is to the bottom-right of the origin square.
                else if(from_rank > to_rank && from_file < to_file) {
                    index = origin - (rank_difference * surface.length) + file_difference;
                }

                // The destination square is to the bottom-left of the origin square.
                else if(from_rank > to_rank && from_file > to_file) {
                    index = origin - (rank_difference * surface.length) - file_difference;
                }

                // The destination square is to the top-left of the origin square.
                else /* if(from_rank > to_rank && from_file < to_file) */ {
                    index = origin + (rank_difference * surface.length) - file_difference;
                }

                // If a piece is present, then the path is obstructed.
                if(surface[index].container) {
                    return true;
                }

                // Decrement to the next square.
                rank_difference--;
                file_difference--;
            }

            return false;
        }

        bool rook(
            const bongcloud::board& surface,
            const std::size_t from,
            const std::size_t to,
            std::size_t difference) {

            while(difference > 0) {
                std::size_t index = (from > to) ? from - difference : from + difference;
                if(surface[index].container) {
                    return true;
                }

                difference -= (difference >= surface.length) ? surface.length : 1;
            }

            return false;
        }
    }
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
            std::size_t difference = internal::absdiff(from, to);
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
                (internal::absdiff(m_last_move->first, m_last_move->second) / length == 2) &&
                (internal::absdiff(from, m_last_move->second) == 1) &&
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
            std::size_t rank_difference = internal::absdiff(from_rank, to_rank);
            std::size_t file_difference = internal::absdiff(from_file, to_file);

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
            std::size_t rank_difference = internal::absdiff(from_rank, to_rank);
            std::size_t file_difference = internal::absdiff(from_file, to_file);
            if(rank_difference != file_difference) {
                return false;
            }

            // Since diagonality was checked, all that's left to do is check for obstacles.
            return !internal::obstructions::bishop(
                *this,
                from,
                from_rank,
                from_file,
                to_rank,
                to_file,
                rank_difference - 1,
                file_difference - 1
            );
        }

        case piece::type_t::rook: {
            std::size_t from_rank = from / length;
            std::size_t from_file = from % length;
            std::size_t to_rank = to / length;
            std::size_t to_file = to % length;

            // Bishops can move diagonally, therefore these differences must be equal.
            std::size_t rank_difference = internal::absdiff(from_rank, to_rank);
            std::size_t file_difference = internal::absdiff(from_file, to_file);
            if(rank_difference != 0 && file_difference != 0) {
                return false;
            }

            // Rooks can only travel in a straight, unobstructed line.
            // Subtract to start obstruction checking from the previous square.
            std::size_t difference = internal::absdiff(from, to);
            difference -= (difference >= length) ? length : 1;
            return !internal::obstructions::rook(*this, from, to, difference);
        }

        case piece::type_t::queen: {
            std::size_t from_rank = from / length;
            std::size_t from_file = from % length;
            std::size_t to_rank = to / length;
            std::size_t to_file = to % length;

            // Queens are equivalent to a combined bishop and rook.
            std::size_t rank_difference = internal::absdiff(from_rank, to_rank);
            std::size_t file_difference = internal::absdiff(from_file, to_file);
            bool diagonal = rank_difference == file_difference;
            bool straight = rank_difference == 0 || file_difference == 0;

            if(!diagonal && !straight) {
                return false;
            } else if(diagonal) {
                return !internal::obstructions::bishop(
                    *this,
                    from,
                    from_rank,
                    from_file,
                    to_rank,
                    to_file,
                    rank_difference - 1,
                    file_difference - 1
                );
            } else /* if(straight) */ {
                std::size_t difference = internal::absdiff(from, to);
                difference -= (difference >= length) ? length : 1;
                return !internal::obstructions::rook(*this, from, to, difference);
            }
        }

        case piece::type_t::king: {
            return true;
        }

        // What piece is this??
        default:
            throw std::runtime_error("illegal piece type");
    }
}
