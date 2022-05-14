#include "ai.hpp"

std::optional<bongcloud::move> bongcloud::random_ai::generate(const bongcloud::board& board) {
    bongcloud::board local = board;
    std::vector<move> moves;

    for(std::size_t from = 0; from < local.length * local.length; from++) {
        for(std::size_t to = 0; to < local.length * local.length; to++) {
            if(!this->movable(from, to, board)) {
                continue;
            }

            if(local.mutate(from, to)) {
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
