#pragma once

#include "board.hpp"

#include <cstddef>
#include <random>

namespace bongcloud {
    class random_ai {
        public:
            // The default constructor.
            random_ai(const bongcloud::board&);

            // Generates a pair of integers representing a random move. May or may not be legal.
            move generate(void);

        private:
            // A (hopefully) true random number generator.
            std::random_device m_device;

            // A pseudo-random number generator.
            std::minstd_rand m_random;

            // The distribution object, which limits the range & type of number produced.
            std::uniform_int_distribution<std::size_t> m_distribution;
    };
}
