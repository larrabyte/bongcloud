#include "pieces.hpp"
#include "board.hpp"

#include <centurion.hpp>
#include <fmt/core.h>
#include <cstddef>

// bongcloud::square constructors.
bongcloud::square::square(void) : container(std::nullopt) {}
bongcloud::square::square(const bongcloud::piece& p) : container(p) {}

bongcloud::board::board(const std::size_t l) : length {l}, m_internal {l * l, square()} {
    fmt::print("[bongcloud] initialising board of size {}x{}...\n", l, l);
}

bool bongcloud::board::move(const std::size_t from, const std::size_t to) {
    fmt::print("[bongcloud] moving piece from square {} to square {}\n", from, to);

    // Check if piece movement rules are being violated.
    if(!is_movement_allowed(from, to)) {
        return false;
    }

    // Empty the origin square and move its piece to the destination square.
    m_internal[to].container = m_internal[from].container;
    m_internal[from].container = std::nullopt;
    m_last_move = std::make_pair(from, to);
    return true;
}

void bongcloud::board::load_fen(const std::string_view string) {
    // FEN strings start from the A1 square.
    std::size_t square = 0, index = 0;

    while(index < string.size()) {
        const char c = string[index++];

        switch(c) {
            using piece = bongcloud::piece;
            using color = bongcloud::piece::color_t;
            using type = bongcloud::piece::type_t;

            // Lowercase letters represent white pieces, uppercase letters represent black pieces.
            case 'r': m_internal[square++].container = piece(color::white, type::rook); break;
            case 'n': m_internal[square++].container = piece(color::white, type::knight); break;
            case 'b': m_internal[square++].container = piece(color::white, type::bishop); break;
            case 'q': m_internal[square++].container = piece(color::white, type::queen); break;
            case 'k': m_internal[square++].container = piece(color::white, type::king); break;
            case 'p': m_internal[square++].container = piece(color::white, type::pawn); break;
            case 'R': m_internal[square++].container = piece(color::black, type::rook); break;
            case 'N': m_internal[square++].container = piece(color::black, type::knight); break;
            case 'B': m_internal[square++].container = piece(color::black, type::bishop); break;
            case 'Q': m_internal[square++].container = piece(color::black, type::queen); break;
            case 'K': m_internal[square++].container = piece(color::black, type::king); break;
            case 'P': m_internal[square++].container = piece(color::black, type::pawn); break;

            // Numbers signify the number of squares to skip.
            case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                square += c - '0';
                break;

            case '/': // A slash moves the cursor to the next rank.
                square = (((square - 1) / length) * length) + length;
                break;

            case ' ': // All pieces have been placed.
                return;

            default: // Unknown character, abort!
                throw std::runtime_error("illegal FEN string");
        }
    }
}

std::optional<std::pair<std::size_t, std::size_t>> bongcloud::board::last_move(void) const {
    return m_last_move;
}
