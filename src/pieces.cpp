#include "pieces.hpp"
#include "board.hpp"

#include <fmt/core.h>

namespace internal {
    inline std::size_t absdiff(const std::size_t a, const std::size_t b) noexcept {
        return (a > b) ? a - b : b - a;
    }

    namespace obstructions {
        bool bishop(
            const bongcloud::board& board,
            const std::size_t origin,
            const std::size_t from_rank,
            const std::size_t from_file,
            const std::size_t to_rank,
            const std::size_t to_file,
            std::size_t rank_difference,
            std::size_t file_difference) noexcept {

            while(rank_difference > 0 && file_difference > 0) {
                std::size_t index;

                // The destination square is to the top-right of the origin square.
                if(from_rank < to_rank && from_file < to_file) {
                    index = origin + (rank_difference * board.length) + file_difference;
                }

                // The destination square is to the bottom-right of the origin square.
                else if(from_rank > to_rank && from_file < to_file) {
                    index = origin - (rank_difference * board.length) + file_difference;
                }

                // The destination square is to the bottom-left of the origin square.
                else if(from_rank > to_rank && from_file > to_file) {
                    index = origin - (rank_difference * board.length) - file_difference;
                }

                // The destination square is to the top-left of the origin square.
                else /* if(from_rank > to_rank && from_file < to_file) */ {
                    index = origin + (rank_difference * board.length) - file_difference;
                }

                // If a piece is present, then the path is obstructed.
                if(board[index].piece) {
                    return true;
                }

                // Decrement to the next square.
                rank_difference--;
                file_difference--;
            }

            return false;
        }

        bool rook(
            const bongcloud::board& board,
            const std::size_t from,
            const std::size_t to,
            std::size_t difference) noexcept {

            while(difference > 0) {
                std::size_t index = (from > to) ? from - difference : from + difference;
                if(board[index].piece) {
                    return true;
                }

                difference -= (difference >= board.length) ? board.length : 1;
            }

            return false;
        }
    }
}

bool bongcloud::board::permissible(const std::size_t from, const std::size_t to) const {
    // Some initial sanity checks.
    const auto& origin = m_internal[from].piece;
    const auto& dest = m_internal[to].piece;

    if(!origin) {
        throw std::runtime_error("tried to move from square with no piece");
    }

    if(from == to) {
        throw std::runtime_error("tried to check permissiblity of a non-move");
    }

    switch(origin->type) {
        case piece::types::pawn: {
            // Pawns can't move backwards or sideways.
            bool correct_direction = {
                (origin->color == piece::colors::white && from < to) ||
                (origin->color == piece::colors::black && from > to)
            };

            if(!correct_direction) {
                return false;
            }

            // Pawns can move one square forward or two if they're on the first or second-last rank.
            // Since ranks are zero-indexed, this corresponds to the 1st and (length - 2)th ranks.
            // Pawns cannot take directly in-front of them.
            std::size_t difference = internal::absdiff(from, to);
            bool one_forward = difference == length && !m_internal[to].piece;

            bool two_forward = {
                (difference == 2 * length) &&
                (from / length == 1 || from / length == length - 2) &&
                (!m_internal[to].piece)
            };

            // Pawns can diagonally capture if there is a piece present.
            bool diagonal_capture = {
                (difference == length - 1 || difference == length + 1) &&
                (m_internal[to].piece)
            };

            // HON HON!
            bool en_passant = {
                (m_latest) &&
                (m_internal[m_latest->second].piece->type == piece::types::pawn) &&
                (internal::absdiff(m_latest->first, m_latest->second) / length == 2) &&
                (internal::absdiff(from, m_latest->second) == 1) &&
                ((origin->color == piece::colors::white && to == m_latest->second + length) ||
                (origin->color == piece::colors::black && to == m_latest->second - length))
            };

            return one_forward || two_forward || diagonal_capture || en_passant;
        }

        case piece::types::knight: {
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

        case piece::types::bishop: {
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

        case piece::types::rook: {
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

        case piece::types::queen: {
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

        case piece::types::king: {
            // The king can move in any direction for one square.
            const std::array<std::size_t, 4> allowed = {1, length - 1, length, length + 1};
            std::size_t difference = internal::absdiff(from, to);

            // std::find() will return allowed.end() if difference is not present in the array.
            return std::find(allowed.begin(), allowed.end(), difference) != allowed.end();
        }

        // What piece is this??
        default:
            throw std::runtime_error("illegal piece type");
    }
}
