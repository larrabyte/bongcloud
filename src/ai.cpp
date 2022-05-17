#include "extras.hpp"
#include "ai.hpp"

#include <fmt/core.h>

namespace internal {
    constexpr double piece_values[] = {
        1.0, // piece::type::pawn
        3.0, // piece::type::knight
        3.0, // piece::type::bishop
        5.0, // piece::type::rook
        9.0, // piece::type::queen
        0.0  // piece::type::king
    };

    // The average position has about 40 legal moves.
    constexpr std::size_t reserve_buffer = 40;
}

namespace bongcloud { // Implementation of classical_ai.
    double classical_ai::evaluate(const bongcloud::board& board) const {
        double evaluation = 0.0;

        for(const auto& piece : board) {
            if(piece) {
                auto index = ext::to_underlying(piece->variety);
                auto value = internal::piece_values[index];
                evaluation += (piece->hue == piece::color::white) ? value : -value;
            }
        }

        return evaluation;
    }

    std::vector<move> classical_ai::moves(bongcloud::board& board) const {
        // Preallocate space here so we don't spend time resizing and copying.
        std::vector<move> moves;
        moves.reserve(internal::reserve_buffer);

        bool white = board.color() == piece::color::white;
        const auto& cache = board.cache();
        const auto& pieces = (white) ? cache.pieces.white : cache.pieces.black;
        // const auto& targets = (white) ? cache.pieces.black : cache.pieces.white;

        for(const auto piece : pieces) {
            for(std::size_t to = 0; to < board.length * board.length; ++to) {
                if(this->movable(piece, to, board) && board.move(piece, to)) {
                    move m = {piece, to};
                    moves.push_back(m);
                    board.undo();
                }
            }
        }

        return moves;
    }

    double classical_ai::minimax(bongcloud::board& board, double alpha, double beta, const std::size_t depth, const piece::color color) const {
        if(depth == 0) {
            return this->evaluate(board);
        }

        double best;
        piece::color next;

        if(color == piece::color::white) {
            best = -std::numeric_limits<double>::infinity();
            next = piece::color::black;

            for(const auto& move : this->moves(board)) {
                board.move(move.from, move.to);
                auto contender = this->minimax(board, alpha, beta, depth - 1, next);
                best = std::max(best, contender);
                board.undo();

                alpha = std::max(alpha, contender);
                if(beta <= alpha) {
                    break;
                }
            }
        }

        else {
            best = std::numeric_limits<double>::infinity();
            next = piece::color::white;

            for(const auto& move : this->moves(board)) {
                board.move(move.from, move.to);
                auto contender = this->minimax(board, alpha, beta, depth - 1, next);
                best = std::min(best, contender);
                board.undo();

                beta = std::min(beta, contender);
                if(beta <= alpha) {
                    break;
                }
            }
        }

        return best;
    }

    std::optional<move> classical_ai::generate(const bongcloud::board& board) {
        using position = std::pair<move, double>;
        bongcloud::board local = board;
        std::vector<position> moves;
        moves.reserve(internal::reserve_buffer);

        for(const auto& move : this->moves(local)) {
            local.move(move.from, move.to);
            double inf = std::numeric_limits<double>::infinity();
            double score = this->minimax(local, -inf, inf, m_depth, board.color());

            position pair = std::make_pair(move, score);
            moves.push_back(pair);
            local.undo();
        }

        if(moves.empty()) {
            return std::nullopt;
        }

        auto whitesort = [](const position& lhs, const position& rhs) { return lhs.second > rhs.second; };
        auto blacksort = [](const position& lhs, const position& rhs) { return lhs.second < rhs.second; };
        std::sort(moves.begin(), moves.end(), (board.color() == piece::color::white) ? whitesort : blacksort);
        return moves[0].first;
    }
}

namespace bongcloud { // Implementation of random_ai.
    double random_ai::evaluate(const bongcloud::board&) const {
        return 0.0;
    }

    std::optional<move> random_ai::generate(const bongcloud::board& board) {
        bongcloud::board local = board;
        std::vector<move> moves;
        moves.reserve(internal::reserve_buffer);

        for(std::size_t from = 0; from < local.length * local.length; ++from) {
            for(std::size_t to = 0; to < local.length * local.length; ++to) {
                if(this->movable(from, to, local) && local.move(from, to)) {
                    move m = {from, to};
                    moves.push_back(m);
                    local.undo();
                }
            }
        }

        if(moves.empty()) {
            return std::nullopt;
        }

        std::uniform_int_distribution<std::size_t> distribution(0, moves.size() - 1);
        std::size_t index = distribution(m_random);
        return moves[index];
    }
}
