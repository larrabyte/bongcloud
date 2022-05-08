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
                if(board[index]) {
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
                if(board[index]) {
                    return true;
                }

                difference -= (difference >= board.length) ? board.length : 1;
            }

            return false;
        }
    }
}

std::optional<bongcloud::piece::move> bongcloud::board::permissible(const std::size_t from, const std::size_t to) const {
    // Some initial sanity checks.
    const auto& origin = m_internal[from];
    const auto& dest = m_internal[to];

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

    if(origin->variety == piece::type::pawn) {
        // Pawns can't move backwards or sideways.
        bool white_direction = origin->hue == piece::color::white && from_rank < to_rank;
        bool black_direction = origin->hue == piece::color::black && from_rank > to_rank;
        if(!white_direction && !black_direction) {
            return std::nullopt;
        }

        // Pawns can move one square forward or two if it's their first move.
        // Pawns cannot take directly in-front of them.
        bool step_forward = rank_difference == 1 && file_difference == 0 && !dest;
        bool jump_forward = rank_difference == 2 && file_difference == 0 && !dest && origin->moves == 0;
        bool promotion = to_rank == 0 || to_rank == length - 1;

        if(step_forward && promotion) {
            return piece::move::promotion;
        } else if(step_forward || jump_forward) {
            return piece::move::normal;
        }

        // Pawns can also diagionally capture if there is a piece present.
        if(rank_difference == 1 && file_difference == 1 && dest) {
            return piece::move::capture;
        }

        // HON HON!
        const auto& last = latest();

        if(last) {
            bool pawn = m_internal[last->to]->variety == piece::type::pawn;
            bool jumped = internal::absdiff(last->from, last->to) == length * 2;
            bool taking = internal::absdiff(to, last->to) == length;
            bool adjacent = internal::absdiff(from, last->to) == 1;
            if(pawn && jumped && taking && adjacent) {
                return piece::move::en_passant;
            }
        }

        // If we are here, then the move must be illegal.
        return std::nullopt;
    }

    else if(origin->variety == piece::type::knight) {
        // Knights can move in an L-shape.
        bool type_one = rank_difference == 1 && file_difference == 2;
        bool type_two = rank_difference == 2 && file_difference == 1;

        if(type_one || type_two) {
            return (dest) ? piece::move::capture : piece::move::normal;
        }

        return std::nullopt;
    }

    else if(origin->variety == piece::type::bishop) {
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
            return (dest) ? piece::move::capture : piece::move::normal;
        }

        return std::nullopt;
    }

    else if(origin->variety == piece::type::rook) {
        if(rank_difference != 0 && file_difference != 0) {
            return std::nullopt;
        }

        // Rooks can only travel in a straight, unobstructed line.
        // Subtract to start obstruction checking from the previous square.
        std::size_t difference = internal::absdiff(from, to);
        difference -= (difference >= length) ? length : 1;
        bool obstructed = internal::obstructions::rook(*this, from, to, difference);

        if(!obstructed) {
            return (dest) ? piece::move::capture : piece::move::normal;
        }

        return std::nullopt;
    }

    else if(origin->variety == piece::type::queen) {
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
            return (dest) ? piece::move::capture : piece::move::normal;
        }

        return std::nullopt;
    }

    else if(origin->variety == piece::type::king) {
        // The king can move in any direction (but only for one square).
        bool horizontal = rank_difference == 0 && file_difference == 1;
        bool vertical = rank_difference == 1 && file_difference == 0;
        bool diagonal = rank_difference == 1 && file_difference == 1;

        if(horizontal || vertical || diagonal) {
            return (dest) ? piece::move::capture : piece::move::normal;
        }

        // The king can also castle short (king-side).
        bool castle_short = from < to;
        std::size_t castle_index = (castle_short) ? to + 1 : to - 2;

        if(castle_index < length * length) {
            const auto &target = m_internal[castle_index];
            std::size_t castle_difference = internal::absdiff(from, castle_index);

            bool king_ready = origin->moves == 0;
            bool jump_to_empty = rank_difference == 0 && file_difference == 2 && !dest;
            bool rook_ready = target && target->variety == piece::type::rook && target->moves == 0;
            bool obstructed = internal::obstructions::rook(*this, from, castle_index, castle_difference - 1);

            if(king_ready && jump_to_empty && rook_ready && !obstructed) {
                return (castle_short) ? piece::move::short_castle : piece::move::long_castle;
            }
        }

        return std::nullopt;
    }

    else {
        throw std::runtime_error("illegal piece type");
    }
}
