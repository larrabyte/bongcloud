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

bongcloud::ai::ai(const std::size_t depth) noexcept : m_depth(depth) {
    fmt::print("[bongcloud] AI search depth set to {} ply.\n", m_depth);
}

double bongcloud::ai::evaluate(bongcloud::board& board) const {
    piece::color color = board.color();
    double evaluation = 0.0;

    if(board.check(color) && this->moves(board).empty()) {
        // Checkmate is the best outcome!
        evaluation = std::numeric_limits<double>::infinity();
        return (color == piece::color::white) ? evaluation : -evaluation;
    }

    for(const auto& piece : board) {
        if(piece) {
            auto index = ext::to_underlying(piece->variety);
            auto value = internal::piece_values[index];
            evaluation += (piece->hue == piece::color::white) ? value : -value;
        }
    }

    return evaluation;
}

std::optional<bongcloud::move> bongcloud::ai::generate(const bongcloud::board& board) const {
    // Create a local copy so that we don't modify the passed in board
    // and have the renderer go crazy trying to render the AI's moves.
    bongcloud::board local = board;

    using position = std::pair<move, double>;
    std::vector<position> moves;
    moves.reserve(internal::reserve_buffer);

    for(const auto& move : this->moves(local)) {
        // Make each move and then determine its score through the minimax algorithm.
        local.move(move.from, move.to);
        double inf = std::numeric_limits<double>::infinity();
        double score = this->minimax(local, -inf, inf, m_depth, local.color());

        position pair = std::make_pair(move, score);
        moves.push_back(pair);
        local.undo();
    }

    if(moves.empty()) {
        return std::nullopt;
    }

    std::sort(moves.begin(), moves.end(), [](const position& lhs, const position& rhs) {
        return lhs.second > rhs.second;
    });

    std::size_t index = (board.color() == piece::color::white) ? 0 : moves.size() - 1;
    return moves[index].first;
}

std::size_t bongcloud::ai::perft(const bongcloud::board& board, const std::size_t n) const {
    bongcloud::board local = board;
    return this->positions(local, n);
}

double bongcloud::ai::minimax(bongcloud::board& board, double alpha, double beta, const std::size_t depth, const piece::color color) const {
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

std::size_t bongcloud::ai::positions(bongcloud::board& board, const std::size_t depth) const {
    if(depth == 0) {
        return 1;
    }

    std::size_t count = 0;
    for(const auto& move : this->moves(board)) {
        board.move(move.from, move.to);
        count += this->positions(board, depth - 1);
        board.undo();
    }

    return count;
}

std::vector<bongcloud::move> bongcloud::ai::moves(bongcloud::board& board) const {
    // Preallocate space here so we don't spend time resizing and copying.
    std::vector<move> moves;
    moves.reserve(internal::reserve_buffer);

    for(std::size_t from = 0; from < board.length * board.length; ++from) {
        for(std::size_t to = 0; to < board.length * board.length; ++to) {
            if(from != to && board[from] && board.move(from, to)) {
                move m = {from, to};
                moves.push_back(m);
                board.undo();
            }
        }
    }

    return moves;
}
