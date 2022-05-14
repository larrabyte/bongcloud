#pragma once

#include "board.hpp"

#include <cstddef>
#include <random>

namespace bongcloud {
    class ai {
        public:
            // Returns a floating-point number representing the advantage for a certain player.
            // Positive means an advantage for white, while negative means an advantage for black.
            virtual double evaluate(const bongcloud::board&) { return 0.0; };

            // Generates a legal move for the current board's player.
            virtual std::optional<move> generate(const bongcloud::board&) = 0;

            // An empty destructor implementation.
            virtual ~ai() {};

        protected:
            // Common subroutine for determining whether a move is trivially allowed.
            inline bool movable(const std::size_t f, const std::size_t t, const board& b) {
                return f != t && b[f] && b[f]->hue == b.color() && (!b[t] || b[t]->hue != b.color());
            }
    };

    class random_ai final : public ai {
        public:
            random_ai(const bongcloud::board&) : m_random {m_device()} {};
            std::optional<move> generate(const bongcloud::board&);

        private:
            // A (hopefully) true random number generator.
            std::random_device m_device;

            // A pseudo-random number generator.
            std::minstd_rand m_random;
    };

    class classical_ai final : public ai {
        public:
            double evaluate(const bongcloud::board&);
            std::optional<move> generate(const bongcloud::board&);
    };
}
