#include "ai.hpp"

#include <fmt/core.h>

bongcloud::random_ai::random_ai(const bongcloud::board& b) :
    m_device {std::random_device()},
    m_random {m_device()},
    m_distribution (0, (b.length * b.length) - 1) {

    fmt::print("[bongcloud] initialising random ai...\n");
}

bongcloud::move bongcloud::random_ai::generate(void) {
    auto a = m_distribution(m_random);
    auto b = m_distribution(m_random);
    return {a, b};
}
