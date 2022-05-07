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

bool bongcloud::board::check(const piece::colors color) const {
    // Attempt to find the index of the specified player's king.
    std::optional<std::size_t> king;

    for(std::size_t i = 0; i < length * length; i++) {
        const auto& piece = m_internal[i].piece;
        if(piece && piece->color == color && piece->type == piece::types::king) {
            king = i;
            break;
        }
    }

    // Invalid sentintel still remains.
    if(!king) {
        throw std::runtime_error("no king on board");
    }

    // Find the index of the current player's king.
    for(std::size_t i = 0; i < length * length; i++) {
        const auto& piece = m_internal[i].piece;
        if(piece && piece->color != m_color && permissible(i, *king)) {
            return true;
        }
    }

    return false;
}

bool bongcloud::board::move(const std::size_t from, const std::size_t to) {
    auto& origin = m_internal[from].piece;
    auto& dest = m_internal[to].piece;

    if(!origin) {
        throw std::runtime_error("tried to move from square with no piece");
    }

    bool correct_color = origin->color == m_color;
    bool cannibal = dest && origin->color == dest->color;
    auto type = permissible(from, to);

    if(m_anarchy || (correct_color && !cannibal && type)) {
        if(type == piece::moves::normal || type == piece::moves::capture) {
            // Move and clear.
            dest = origin;
            dest->move_count++;
            origin = std::nullopt;
        }

        else if(type == piece::moves::en_passant) {
            // Move the piece at the origin square to the
            // destination square and update its move count.
            dest = origin;
            dest->move_count++;

            // Clear the origin square and the square directly behind,
            // which is equivalent to a square adjacent to the origin.
            auto& target = m_internal[m_latest->second].piece;
            origin = std::nullopt;
            target = std::nullopt;
        }

        else if(type == piece::moves::short_castle || type == piece::moves::long_castle) {
            // Calculate the appropriate squares to send the king and rook.
            std::size_t origin_offset = (type == piece::moves::short_castle) ? to + 1 : to - 2;
            std::size_t dest_offset = (type == piece::moves::short_castle) ? to - 1 : to + 1;
            auto& rook_origin = m_internal[origin_offset].piece;
            auto& rook_dest = m_internal[dest_offset].piece;

            // Update piece positions and increment the move count.
            dest = origin;
            rook_dest = rook_origin;
            dest->move_count++;
            rook_dest->move_count++;

            // Remove the original king and rook from the board.
            origin = std::nullopt;
            rook_origin = std::nullopt;
        }

        else {
            throw std::runtime_error("unimplemented movement type");
        }

        // Update the latest move.
        m_latest = std::make_pair(from, to);

        // Update m_color to reflect the next player to move.
        auto index = static_cast<std::size_t>(m_color);
        const std::array<piece::colors, 2> next {
            piece::colors::black,
            piece::colors::white
        };

        m_color = next[index];
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
