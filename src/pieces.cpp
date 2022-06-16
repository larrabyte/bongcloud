#include "pieces.hpp"
#include "extras.hpp"
#include "board.hpp"

#include <fmt/core.h>

namespace internal {
    std::size_t absdiff(const std::size_t a, const std::size_t b) noexcept {
        return (a > b) ? a - b : b - a;
    }

    bool bishop(const bcl::board& board, const std::size_t from, const std::size_t to) noexcept {
        assert(from != to);

        bcl::index source = {from / board.length, from % board.length};
        bcl::index sink = {to / board.length, to % board.length};

        // Make sure that the bishop is moving diagonally.
        assert(
            internal::absdiff(source.rank, sink.rank) ==
            internal::absdiff(source.file, sink.file)
        );

        // The destination square is to the top-right of the origin square.
        if(source.rank < sink.rank && source.file < sink.file) {
            for(std::size_t index = from + board.length + 1; index != to; index += board.length + 1) {
                if(board[index]) {
                    return true;
                }
            }
        }

        // The destination square is to the bottom-right of the origin square.
        else if(source.rank > sink.rank && source.file < sink.file) {
            for(std::size_t index = from - board.length + 1; index != to; index -= board.length - 1) {
                if(board[index]) {
                    return true;
                }
            }
        }

        // The destination square is to the bottom-left of the origin square.
        else if(source.rank > sink.rank && source.file > sink.file) {
            for(std::size_t index = from - board.length - 1; index != to; index -= board.length + 1) {
                if(board[index]) {
                    return true;
                }
            }
        }

        // The destination square is to the top-left of the origin square.
        else {
            for(std::size_t index = from + board.length - 1; index != to; index += board.length - 1) {
                if(board[index]) {
                    return true;
                }
            }
        }

        return false;
    }

    bool rook(const bcl::board& board, const std::size_t from, const std::size_t to) noexcept {
        std::size_t difference = internal::absdiff(from, to);
        std::size_t subtractor = (difference >= board.length) ? board.length : 1;

        assert(from != to);
        assert(difference < board.length || difference % board.length == 0);

        if(from > to) {
            while((difference -= subtractor) > 0) {
                if(board[from - difference]) {
                    return true;
                }
            }
        }

        else {
            while((difference -= subtractor) > 0) {
                if(board[from + difference]) {
                    return true;
                }
            }
        }

        return false;
    }
}

std::optional<bcl::piece::move> bcl::board::pseudolegal(const std::size_t from, const std::size_t to) const noexcept {
    const auto& origin = m_internal[from];
    const auto& dest = m_internal[to];

    // Some sanity checks.
    assert(origin.has_value());
    assert(from != to);

    index difference = {
        internal::absdiff(from / length, to / length),
        internal::absdiff(from % length, to % length)
    };

    switch(origin->variety) {
        case piece::type::pawn: {
            bool forward = {
                (origin->hue == piece::color::white && from < to) ||
                (origin->hue == piece::color::black && from > to)
            };

            if(!forward) {
                // Pawns can't move backwards.
                return std::nullopt;
            }

            index source = {from / length, from % length};

            bool pushing = {
                (source.rank == 1 || source.rank == length - 2) &&
                difference.rank == 2 && difference.file == 0 && !dest
            };

            if(pushing) {
                // Pawns can move two squares forward on their first move, assuming a clear path.
                bool white = (origin->hue == piece::color::white);
                std::size_t adjacent = (white) ? from + length : from - length;
                if(!m_internal[adjacent]) {
                    return piece::move::normal;
                }

                return std::nullopt;
            }

            index sink = {to / length, to % length};

            if(difference.rank == 1 && difference.file == 0 && !dest) {
                // Pawns can always move one square forward if not blocked.
                // If a pawn has managed to reach the end of the board, it is promoted instead.
                if(sink.rank == 0 || sink.rank == length - 1) {
                    return piece::move::promotion;
                }

                return piece::move::normal;
            }

            if(difference.rank == 1 && difference.file == 1 && dest) {
                // Pawns can also diagonally capture if there is a piece present.
                if(sink.rank == 0 || sink.rank == length - 1) {
                    return piece::move::promotion;
                }

                return piece::move::capture;
            }

            if(const auto& latest = this->latest()) {
                // En-passant capture is possible only if...
                // - The last move was a 2-square pawn move.
                // - The target square is behind the pawn to be captured.
                // - The attacking pawn is adjacent to the target pawn.
                bool takable = {
                    m_internal[latest->to]->variety == piece::type::pawn &&
                    internal::absdiff(latest->from, latest->to) == length * 2 &&
                    internal::absdiff(to, latest->to) == length &&
                    internal::absdiff(from, latest->to) == 1 &&
                    !(source.file == 0 && sink.file == length - 1) &&
                    !(source.file == length - 1 && sink.file == 0)
                };

                if(takable) {
                    return piece::move::en_passant;
                }
            }

            break;
        }

        case piece::type::knight: {
            // Knights can move in an L-shape (2 units in one direction, 1 unit in the other).
            bool allowed = {
                (difference.rank == 1 && difference.file == 2) ||
                (difference.rank == 2 && difference.file == 1)
            };

            if(allowed) {
                return (dest) ? piece::move::capture : piece::move::normal;
            }

            break;
        }

        case piece::type::bishop: {
            // Make sure the bishop is moving diagonally and not obstructed.
            if(difference.rank == difference.file && !internal::bishop(*this, from, to)) {
                return (dest) ? piece::move::capture : piece::move::normal;
            }

            break;
        }

        case piece::type::rook: {
            // Make sure the rook is travelling in a straight, unobstructed line.
            if((difference.rank == 0 || difference.file == 0) && !internal::rook(*this, from, to)) {
                return (dest) ? piece::move::capture : piece::move::normal;
            }

            break;
        }

        case piece::type::queen: {
            // If the queen is moving in a diagonal pattern, it must obey bishop movement rules.
            if(difference.rank == difference.file && !internal::bishop(*this, from, to)) {
                return (dest) ? piece::move::capture : piece::move::normal;
            }

            // Otherwise if the queen is moving in a straight line, it must obey rook movement rules.
            if((difference.rank == 0 || difference.file == 0) && !internal::rook(*this, from, to)) {
                return (dest) ? piece::move::capture : piece::move::normal;
            }

            break;
        }

        case piece::type::king: {
            // The king can move in any direction (but only for one square).
            // This is equivalent to ORing the rank and file difference and comparing with 1.
            if((difference.rank | difference.file) == 1) {
                return (dest) ? piece::move::capture : piece::move::normal;
            }

            // The king can also castle.
            if(difference.rank == 0 && difference.file == 2 && !dest) {
                std::size_t left = (m_color == piece::color::white) ? 0 : length * (length - 1);
                std::size_t right = (m_color == piece::color::white) ? length - 1 : (length * length) - 1;

                // The valid destinations are 2 squares to the right of the left-most square,
                // or 1 square to the left of the right-most square (depending on color).
                if(to != left + 2 && to != right - 1) {
                    return std::nullopt;
                };

                const auto& rights = m_rights[origin->hue];
                std::size_t index = (to == left + 2) ? left : right;
                const auto& side = (to == left + 2) ? rights.kingside : rights.queenside;
                const auto& target = m_internal[index];

                bool castleable = {
                    target && target->variety == piece::type::rook &&
                    !internal::rook(*this, from, index) && side
                };

                if(castleable) {
                    return (from < to) ? piece::move::short_castle : piece::move::long_castle;
                }
            }

            break;
        }
    }

    return std::nullopt;
}
