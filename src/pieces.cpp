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

std::optional<bongcloud::piece::moves> bongcloud::board::permissible(const std::size_t from, const std::size_t to) const {
    // Some initial sanity checks.
    const auto& origin = m_internal[from].piece;
    const auto& dest = m_internal[to].piece;

    if(!origin) {
        throw std::runtime_error("tried to move from square with no piece");
    }

    if(from == to) {
        throw std::runtime_error("tried to check permissiblity of a non-move");
    }

    // Apply piece movement rules.
    std::size_t from_rank = from / length;
    std::size_t from_file = from % length;
    std::size_t to_rank = to / length;
    std::size_t to_file = to % length;
    std::size_t rank_difference = internal::absdiff(from_rank, to_rank);
    std::size_t file_difference = internal::absdiff(from_file, to_file);

    if(origin->type == piece::types::pawn) {
        // Pawns can't move backwards or sideways.
        bool white_direction = origin->color == piece::colors::white && from_rank < to_rank;
        bool black_direction = origin->color == piece::colors::black && from_rank > to_rank;
        if(!white_direction && !black_direction) {
            return std::nullopt;
        }

        // Pawns can move one square forward or two if it's their first move.
        // Pawns cannot take directly in-front of them.
        bool step_forward = rank_difference == 1 && file_difference == 0 && !dest;
        bool jump_forward = rank_difference == 2 && file_difference == 0 && !dest && origin->move_count == 0;
        if(step_forward || jump_forward) {
            return piece::moves::normal;
        }

        // Pawns can also diagionally capture if there is a piece present.
        if(rank_difference == 1 && file_difference == 1 && dest) {
            return piece::moves::capture;
        }

        // HON HON!
        if(m_latest) {
            bool pawn = m_internal[m_latest->second].piece->type == piece::types::pawn;
            bool jumped = internal::absdiff(m_latest->first, m_latest->second) == length * 2;
            bool taking = internal::absdiff(to, m_latest->second) == length;
            bool adjacent = internal::absdiff(from, m_latest->second) == 1;
            if(pawn && jumped && taking && adjacent) {
                return piece::moves::en_passant;
            }
        }

        // If we are here, then the move must be illegal.
        return std::nullopt;
    }

    else if(origin->type == piece::types::knight) {
        // Knights can move in an L-shape.
        bool type_one = rank_difference == 1 && file_difference == 2;
        bool type_two = rank_difference == 2 && file_difference == 1;

        if(type_one || type_two) {
            return (dest) ? piece::moves::capture : piece::moves::normal;
        }

        return std::nullopt;
    }

    else if(origin->type == piece::types::bishop) {
        // Bishops can move diagonally, therefore the differences must be equal.
        if(rank_difference != file_difference) {
            return std::nullopt;
        }

        // Since diagonality was checked, all that's left to do is check for obstacles.
        // Subtract one from both differences to start obstruction checking from the previous square.
        bool obstructed = internal::obstructions::bishop(
            *this,
            from,
            from_rank,
            from_file,
            to_rank,
            to_file,
            rank_difference - 1,
            file_difference - 1
        );

        if(!obstructed) {
            return (dest) ? piece::moves::capture : piece::moves::normal;
        }

        return std::nullopt;
    }

    else if(origin->type == piece::types::rook) {
        if(rank_difference != 0 && file_difference != 0) {
            return std::nullopt;
        }

        // Rooks can only travel in a straight, unobstructed line.
        // Subtract to start obstruction checking from the previous square.
        std::size_t difference = internal::absdiff(from, to);
        difference -= (difference >= length) ? length : 1;
        bool obstructed = internal::obstructions::rook(*this, from, to, difference);

        if(!obstructed) {
            return (dest) ? piece::moves::capture : piece::moves::normal;
        }

        return std::nullopt;
    }

    else if(origin->type == piece::types::queen) {
        bool obstructed;

        // If the queen is moving in a diagonal pattern, it must obey bishop movement rules.
        if(rank_difference == file_difference) {
            obstructed = internal::obstructions::bishop(
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

        // Otherwise if the queen is moving in a straight line, it must obey rook movement rules.
        else if(rank_difference == 0 || file_difference == 0) {
            std::size_t difference = internal::absdiff(from, to);
            difference -= (difference >= length) ? length : 1;
            obstructed = internal::obstructions::rook(*this, from, to, difference);
        }

        // A move that is not diagonal or straight is illegal.
        else {
            return std::nullopt;
        }

        if(!obstructed) {
            return (dest) ? piece::moves::capture : piece::moves::normal;
        }

        return std::nullopt;
    }

    else if(origin->type == piece::types::king) {
        // The king can move in any direction (but only for one square).
        bool horizontal = rank_difference == 0 && file_difference == 1;
        bool vertical = rank_difference == 1 && file_difference == 0;
        bool diagonal = rank_difference == 1 && file_difference == 1;

        if(horizontal || vertical || diagonal) {
            return (dest) ? piece::moves::capture : piece::moves::normal;
        }

        // The king can also castle short (king-side).
        bool castle_short = from < to;
        std::size_t castle_index = (castle_short) ? to + 1 : to - 2;

        if(castle_index < length * length) {
            const auto &target = m_internal[castle_index].piece;
            std::size_t castle_difference = internal::absdiff(from, castle_index);

            bool king_ready = origin->move_count == 0;
            bool jump_to_empty = rank_difference == 0 && file_difference == 2 && !dest;
            bool rook_ready = target && target->type == piece::types::rook && target->move_count == 0;
            bool obstructed = internal::obstructions::rook(*this, from, castle_index, castle_difference - 1);

            if(king_ready && jump_to_empty && rook_ready && !obstructed) {
                return (castle_short) ? piece::moves::short_castle : piece::moves::long_castle;
            }
        }

        return std::nullopt;
    }

    else {
        throw std::runtime_error("illegal piece type");
    }
}
