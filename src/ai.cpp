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
        // On average, there are 30 possible legal moves in any given state.
        // We preallocate space here so we don't spend time resizing and copying.
        std::vector<move> moves;
        moves.reserve(internal::reserve_buffer);

        for(std::size_t from = 0; from < board.length * board.length; ++from) {
            for(std::size_t to = 0; to < board.length * board.length; ++to) {
                if(this->movable(from, to, board) && board.mutate(from, to)) {
                    move m = {from, to};
                    moves.push_back(m);
                    board.undo();
                }
            }
        }

        return moves;
    }

    double classical_ai::minimax(bongcloud::board& board, const std::size_t depth, const piece::color color) const {
        if(depth == 0) {
            return this->evaluate(board);
        }

        double worst;
        piece::color next;

        if(color == piece::color::white) {
            worst = -std::numeric_limits<double>::infinity();
            next = piece::color::black;

            for(const auto& move : this->moves(board)) {
                board.mutate(move.from, move.to);
                auto contender = this->minimax(board, depth - 1, next);
                worst = std::max(worst, contender);
                board.undo();
            }
        }

        else {
            worst = std::numeric_limits<double>::infinity();
            next = piece::color::white;

            for(const auto& move : this->moves(board)) {
                board.mutate(move.from, move.to);
                auto contender = this->minimax(board, depth - 1, next);
                worst = std::min(worst, contender);
                board.undo();
            }
        }

        return worst;
    }

    std::optional<move> classical_ai::generate(const bongcloud::board& board) {
        bongcloud::board local = board.duplicate();

        using position = std::pair<move, double>;
        std::vector<position> moves;
        moves.reserve(internal::reserve_buffer);

        for(const auto& move : this->moves(local)) {
            local.mutate(move.from, move.to);
            double score = this->minimax(local, m_depth, board.color());

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
        bongcloud::board local = board.duplicate();
        std::vector<move> moves;
        moves.reserve(internal::reserve_buffer);

        for(std::size_t from = 0; from < local.length * local.length; ++from) {
            for(std::size_t to = 0; to < local.length * local.length; ++to) {
                if(this->movable(from, to, local) && local.mutate(from, to)) {
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
