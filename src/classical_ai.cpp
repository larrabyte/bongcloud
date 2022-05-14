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
}

double bongcloud::classical_ai::evaluate(const bongcloud::board& board) {
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

std::vector<bongcloud::move> bongcloud::classical_ai::moves(const bongcloud::board& board) {
    bongcloud::board local = board;
    std::vector<move> moves;

    for(std::size_t from = 0; from < local.length * local.length; from++) {
        for(std::size_t to = 0; to < local.length * local.length; to++) {
            if(!this->movable(from, to, local)) {
                continue;
            }

            if(local.mutate(from, to)) {
                move m = {from, to};
                moves.push_back(m);
                local.undo();
            }
        }
    }

    return moves;
}

double bongcloud::classical_ai::minimax(bongcloud::board& board, const std::size_t depth, const piece::color color) {
    if(depth == 0) {
        return this->evaluate(board);
    }

    using doubles = std::numeric_limits<double>;
    using colors = piece::color;
    double worst = (color == colors::white) ? -doubles::infinity() : doubles::infinity();
    colors next = (color == colors::white) ? colors::black : colors::white;

    for(const auto& move : this->moves(board)) {
        board.mutate(move.from, move.to);
        double contender = this->minimax(board, depth - 1, next);
        worst = (color == colors::white) ? std::max(worst, contender) : std::min(worst, contender);
        board.undo();
    }

    return worst;
}

std::optional<bongcloud::move> bongcloud::classical_ai::generate(const bongcloud::board& board) {
    using position = std::pair<move, double>;
    std::vector<position> moves;

    for(const auto& move : this->moves(board)) {
        bongcloud::board local = board;
        local.mutate(move.from, move.to);
        double score = this->minimax(local, m_depth, board.color());

        position pair = std::make_pair(move, score);
        moves.push_back(pair);
    }

    if(moves.empty()) {
        return std::nullopt;
    }

    auto whitesort = [](const position& lhs, const position& rhs) { return lhs.second > rhs.second; };
    auto blacksort = [](const position& lhs, const position& rhs) { return lhs.second < rhs.second; };
    std::sort(moves.begin(), moves.end(), (board.color() == piece::color::white) ? whitesort : blacksort);
    return moves[0].first;
}
