#pragma once

#include "board.hpp"

#include <optional>
#include <cstddef>
#include <future>

namespace bcl {
    class ai {
        public:
            // All functions that take non-constant board references will utilise
            // the passed in board as a scratch area - however, all modifications
            // performed will be undone before returning.
            ai(const std::size_t, const bool) noexcept;

            // Returns a floating-point number representing the advantage for a certain player.
            // Positive means an advantage for white, while negative means an advantage for black.
            double evaluate(board&) const noexcept;

            // Generates a legal move for the current board's player.
            std::optional<move> generate(const board&) const noexcept;

            // Returns the number of legal moves after n ply.
            std::size_t perft(const board&, const std::size_t) const noexcept;

            // The number of layers to search when generating a move.
            const std::size_t layers;

            // A future for executing expensive operations in a separate thread.
            std::future<std::optional<move>> future;

            // Whether the AI is enabled.
            bool enabled;

        private:
            // An implementation of the minimax algorithm.
            double minimax(board&, double, double, const std::size_t, const piece::color) const noexcept;
    };
}
