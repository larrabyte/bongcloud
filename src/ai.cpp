#include "extras.hpp"
#include "ai.hpp"

#include <fmt/core.h>

bcl::ai::ai(const std::size_t s, const bool e) noexcept : layers {s}, enabled {e} {
    if(e) {
        fmt::print("[bongcloud] AI enabled, search depth set to {} ply.\n", s);
    }
}

double bcl::ai::evaluate(bcl::board& board) const noexcept {
    double evaluation = 0.0;

    if(board.checkmate()) {
        // Checkmate is the best outcome!
        evaluation = std::numeric_limits<double>::infinity();
        return (board.color() == piece::color::white) ? evaluation : -evaluation;
    }

    for(const auto& piece : board) {
        if(piece) {
            auto index = ext::to_underlying(piece->variety);
            auto value = constants::piece_values[index];
            evaluation += (piece->hue == piece::color::white) ? value : -value;
        }
    }

    return evaluation;
}

std::optional<bcl::move> bcl::ai::generate(const bcl::board& board) const noexcept {
    // Create a local copy so that we don't modify the passed in board
    // and have the renderer go crazy trying to render the AI's moves.
    bcl::board local = board;
    std::vector<std::pair<move, double>> moves;
    moves.reserve(constants::move_buffer_reserve);

    for(const auto& move : local.moves()) {
        // Make each move and then determine its score through the minimax algorithm.
        local.move(move.from, move.to);
        double inf = std::numeric_limits<double>::infinity();
        double score = this->minimax(local, -inf, inf, layers, local.color());

        auto pair = std::make_pair(move, score);
        moves.push_back(pair);
        local.undo();
    }

    if(moves.empty()) {
        return std::nullopt;
    }

    std::sort(moves.begin(), moves.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second > rhs.second;
    });

    std::size_t index = (board.color() == piece::color::white) ? 0 : moves.size() - 1;
    return moves[index].first;
}

double bcl::ai::minimax(bcl::board& board, double alpha, double beta, const std::size_t depth, const piece::color color) const noexcept {
    if(depth == 0) {
        return this->evaluate(board);
    }

    double best;
    piece::color next;

    if(color == piece::color::white) {
        best = -std::numeric_limits<double>::infinity();
        next = piece::color::black;

        for(const auto& move : board.moves()) {
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

        for(const auto& move : board.moves()) {
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
