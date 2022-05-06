#include "pieces.hpp"
#include "board.hpp"

#include <centurion.hpp>
#include <fmt/core.h>
#include <cstddef>

bongcloud::board::board(const std::size_t l, const bool anarchy) :
    length {l},
    m_internal {l * l},
    m_latest {std::nullopt},
    m_anarchy {anarchy} {

    fmt::print("[bongcloud] initialising board of size {}x{}... (anarchy: {})\n", l, l, anarchy);
}

void bongcloud::board::print(void) const {
    // Piece-to-character conversion routine.
    auto charconv = [](const std::optional<piece>& p) {
        if(!p) {
            return '-';
        }

        char c;

        switch(p->type) {
            case piece::types::pawn: c = 'p'; break;
            case piece::types::knight: c = 'n'; break;
            case piece::types::bishop: c = 'b'; break;
            case piece::types::rook: c = 'r'; break;
            case piece::types::queen: c = 'q'; break;
            case piece::types::king: c = 'k'; break;
            default: c = '?'; break;
        }

        if(c != '?' && p->color == piece::colors::black) {
            c = std::toupper(c);
        }

        return c;
    };

    // Start from the top-left square.
    std::size_t rank = 7, file = 0;
    bool finished = false;
    fmt::print("[bongcloud] ");

    while(!finished) {
        std::size_t index = (rank * length) + file;
        const auto& piece = m_internal[index].piece;

        if(!piece) {
            fmt::print("-");
        }

        else {
            char c;

            switch(piece->type) {
                case piece::types::pawn: c = 'p'; break;
                case piece::types::knight: c = 'n'; break;
                case piece::types::bishop: c = 'b'; break;
                case piece::types::rook: c = 'r'; break;
                case piece::types::queen: c = 'q'; break;
                case piece::types::king: c = 'k'; break;
                default: c = '?'; break;
            }

            if(c != '?' && piece->color == piece::colors::black) {
                c = std::toupper(c);
            }

            fmt::print("{}", c);
        }

        // Advance the rank and file indices appropriately to move on to the next square/rank.
        if(rank == 0 && file == length - 1) {
            finished = true;
        }

        else if(++file == length) {
            rank--;
            file = 0;
            fmt::print("\n[bongcloud] ");
        }

        else {
            fmt::print(" ");
        }
    }
}

bool bongcloud::board::move(const std::size_t from, const std::size_t to) {
    auto& origin = m_internal[from].piece;
    auto& dest = m_internal[to].piece;

    if(!origin) {
        throw std::runtime_error("tried to move from square with no piece");
    }

    bool correct_color = origin->color == m_color;
    bool cannibal = dest && origin->color == dest->color;
    bool movable = permissible(from, to);

    if(m_anarchy || (correct_color && !cannibal && movable)) {
        // Empty the origin square and move its piece to the destination square.
        dest = origin;
        origin = std::nullopt;
        m_latest = std::make_pair(from, to);
        m_color = (m_color == piece::colors::white) ? piece::colors::black : piece::colors::white;
        return true;
    }

    return false;
}

void bongcloud::board::load_fen(const std::string_view string) {
    // FEN strings start from the A1 square.
    std::size_t square = 0, index = 0;

    while(index < string.size()) {
        const char c = string[index++];

        switch(c) {
            using piece = bongcloud::piece;
            using color = bongcloud::piece::colors;
            using type = bongcloud::piece::types;

            // Lowercase letters represent white pieces, uppercase letters represent black pieces.
            case 'r': m_internal[square++].piece = piece(color::white, type::rook); break;
            case 'n': m_internal[square++].piece = piece(color::white, type::knight); break;
            case 'b': m_internal[square++].piece = piece(color::white, type::bishop); break;
            case 'q': m_internal[square++].piece = piece(color::white, type::queen); break;
            case 'k': m_internal[square++].piece = piece(color::white, type::king); break;
            case 'p': m_internal[square++].piece = piece(color::white, type::pawn); break;
            case 'R': m_internal[square++].piece = piece(color::black, type::rook); break;
            case 'N': m_internal[square++].piece = piece(color::black, type::knight); break;
            case 'B': m_internal[square++].piece = piece(color::black, type::bishop); break;
            case 'Q': m_internal[square++].piece = piece(color::black, type::queen); break;
            case 'K': m_internal[square++].piece = piece(color::black, type::king); break;
            case 'P': m_internal[square++].piece = piece(color::black, type::pawn); break;

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
