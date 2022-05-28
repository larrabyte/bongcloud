#pragma once

#include "board.hpp"

#include <cstddef>

namespace bongcloud {
    class ai {
        public:
            // All functions that take non-constant board references will utilise
            // the passed in board as a scratch area - however, all modifications
            // performed will be undone before returning.
            ai(const std::size_t) noexcept;

            // Returns a floating-point number representing the advantage for a certain player.
            // Positive means an advantage for white, while negative means an advantage for black.
            double evaluate(board&) const;

            // Generates a legal move for the current board's player.
            std::optional<move> generate(const board&) const;

            // Returns the number of legal moves after n ply.
            std::size_t perft(const board&, const std::size_t) const;

        private:
            // An implementation of the minimax algorithm.
            double minimax(board&, double, double, const std::size_t, const piece::color) const;

            // An algorithm that counts possible positions recursively.
            std::size_t positions(board&, const std::size_t) const;

            // Returns a vector containing all possible moves for a given board.
            std::vector<move> moves(board&) const;

            // The number of layers to search when generating a move.
            const std::size_t m_depth;
    };
}
