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
        const auto& piece = m_internal[index];

        if(!piece) {
            fmt::print("-");
        }

        else {
            char c;

            switch(piece->variety) {
                case piece::type::pawn: c = 'p'; break;
                case piece::type::knight: c = 'n'; break;
                case piece::type::bishop: c = 'b'; break;
                case piece::type::rook: c = 'r'; break;
                case piece::type::queen: c = 'q'; break;
                case piece::type::king: c = 'k'; break;
                default: c = '?'; break;
            }

            if(c != '?' && piece->hue == piece::color::black) {
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

bool bongcloud::board::check(const piece::color color) const {
    // Attempt to find the index of the specified player's king.
    std::optional<std::size_t> king;

    for(std::size_t i = 0; i < length * length; i++) {
        const auto& piece = m_internal[i];
        if(piece && piece->hue == color && piece->variety == piece::type::king) {
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
        const auto& piece = m_internal[i];
        if(piece && piece->hue != m_color && permissible(i, *king)) {
            return true;
        }
    }

    return false;
}

bool bongcloud::board::mutate(const std::size_t from, const std::size_t to) {
    auto& origin = m_internal[from];
    auto& dest = m_internal[to];

    if(!origin) {
        throw std::runtime_error("tried to move from square with no piece");
    }

    bool correct_color = origin->hue == m_color;
    bool cannibal = dest && origin->hue == dest->hue;
    auto type = permissible(from, to);

    if(m_anarchy || (correct_color && !cannibal && type)) {
        if(type == piece::move::normal || type == piece::move::capture) {
            // Move and clear.
            dest = origin;
            dest->moves++;
            origin = std::nullopt;
        }

        else if(type == piece::move::en_passant) {
            // Move the piece at the origin square to the
            // destination square and update its move count.
            dest = origin;
            dest->moves++;

            // Clear the origin square and the square directly behind,
            // which is equivalent to a square adjacent to the origin.
            auto& target = m_internal[m_latest->second];
            origin = std::nullopt;
            target = std::nullopt;
        }

        else if(type == piece::move::short_castle || type == piece::move::long_castle) {
            // Calculate the appropriate squares to send the king and rook.
            std::size_t origin_offset = (type == piece::move::short_castle) ? to + 1 : to - 2;
            std::size_t dest_offset = (type == piece::move::short_castle) ? to - 1 : to + 1;
            auto& rook_origin = m_internal[origin_offset];
            auto& rook_dest = m_internal[dest_offset];

            // Update piece positions and increment the move count.
            dest = origin;
            rook_dest = rook_origin;
            dest->moves++;
            rook_dest->moves++;

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
        const std::array<piece::color, 2> next {
            piece::color::black,
            piece::color::white
        };

        m_color = next[index];
        return true;
    }

    return false;
}

void bongcloud::board::load(const std::string_view string) {
    // FEN strings start from the A1 square.
    std::size_t square = 0, index = 0;

    while(index < string.size()) {
        const char c = string[index++];

        switch(c) {
            using piece = bongcloud::piece;
            using color = bongcloud::piece::color;
            using type = bongcloud::piece::type;

            // Lowercase letters represent white pieces, uppercase letters represent black pieces.
            case 'r': m_internal[square++] = piece(color::white, type::rook); break;
            case 'n': m_internal[square++] = piece(color::white, type::knight); break;
            case 'b': m_internal[square++] = piece(color::white, type::bishop); break;
            case 'q': m_internal[square++] = piece(color::white, type::queen); break;
            case 'k': m_internal[square++] = piece(color::white, type::king); break;
            case 'p': m_internal[square++] = piece(color::white, type::pawn); break;
            case 'R': m_internal[square++] = piece(color::black, type::rook); break;
            case 'N': m_internal[square++] = piece(color::black, type::knight); break;
            case 'B': m_internal[square++] = piece(color::black, type::bishop); break;
            case 'Q': m_internal[square++] = piece(color::black, type::queen); break;
            case 'K': m_internal[square++] = piece(color::black, type::king); break;
            case 'P': m_internal[square++] = piece(color::black, type::pawn); break;

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
