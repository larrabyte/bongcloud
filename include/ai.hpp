#pragma once

#include "board.hpp"

#include <cstddef>
#include <random>

namespace bongcloud {
    class ai {
        public:
            // Returns a floating-point number representing the advantage for a certain player.
            // Positive means an advantage for white, while negative means an advantage for black.
            virtual double evaluate(const board&) const = 0;

            // Generates a legal move for the current board's player.
            virtual std::optional<move> generate(const board&) = 0;

            // Returns the number of legal moves after n ply.
            virtual std::size_t perft(const board&, const std::size_t) = 0;

            // An empty destructor implementation.
            virtual ~ai() {};
    };

    class classical_ai final : public ai {
        public:
            classical_ai(const board&, const std::size_t s) noexcept : m_depth {s} {};
            double evaluate(const board&) const;
            std::optional<move> generate(const board&);
            std::size_t perft(const board&, const std::size_t);

        private:
            // An implementation of the minimax algorithm.
            double minimax(board&, double, double, const std::size_t, const piece::color) const;

            // An algorithm that counts possible positions recursively.
            std::size_t positions(board&, const std::size_t) const;

            // Returns a vector containing all possible moves for a given board.
            // This will use the board passed in as a scratch area, however
            // all moves performed will be undone by the time this function returns.
            std::vector<move> moves(board&) const;

            // The number of layers to search when generating a move.
            const std::size_t m_depth;
    };
}
