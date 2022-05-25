#include "pieces.hpp"
#include "extras.hpp"
#include "board.hpp"

#include <centurion.hpp>
#include <fmt/core.h>
#include <cstddef>

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

        // Advance the rank and file indices to move on to the next square.
        if(rank == 0 && file == length - 1) {
            finished = true;
        } else if(++file == length) {
            rank--;
            file = 0;
            fmt::print("\n[bongcloud] ");
        } else {
            fmt::print(" ");
        }
    }

    fmt::print("\n");
}

bool bongcloud::board::check(const piece::color color) const {
    // Attempt to find the index of the specified player's king.
    std::vector<std::size_t> kings;

    for(std::size_t i = 0; i < length * length; ++i) {
        const auto& piece = m_internal[i];
        if(piece && piece->hue == color && piece->variety == piece::type::king) {
            kings.push_back(i);
            break;
        }
    }

    assert(!kings.empty());

    // Return true if any king is in check.
    for(const auto king : kings) {
        for(std::size_t i = 0; i < length * length; ++i) {
            const auto& piece = m_internal[i];
            if(piece && piece->hue != color && this->pseudolegal(i, king)) {
                return true;
            }
        }
    }

    return false;
}

bool bongcloud::board::move(const std::size_t from, const std::size_t to) {
    auto& origin = m_internal[from];
    auto& dest = m_internal[to];

    assert(origin.has_value());

    record latest;
    latest.color = m_color;
    latest.move = {from, to};
    latest.trivials = m_trivials;

    bool correct_color = origin->hue == m_color;
    bool cannibal = dest && origin->hue == dest->hue;
    auto type = this->pseudolegal(from, to);
    bool authorised = correct_color && !cannibal && type;

    if(m_trivials >= 100 || !(m_anarchy || authorised)) {
        // 100 trivial ply (50 moves) is a forced stalemate.
        return false;
    }

    if(m_anarchy) {
        // Since most moves are usually considered impossible, we require
        // different logic for piece movement when anarchy mode is enabled.
        if(dest) {
            latest.capture = {to, *dest};
        }

        dest = origin;
        origin = std::nullopt;
        ++dest->moves;
    }

    else /* if(authorised) */ {
        if(type == piece::move::normal) {
            dest = origin;
            origin = std::nullopt;
            ++dest->moves;
        }

        else if(type == piece::move::capture) {
            latest.capture = {to, *dest};
            dest = origin;
            origin = std::nullopt;
            ++dest->moves;
        }

        else if(type == piece::move::en_passant) {
            dest = origin;
            ++dest->moves;

            // Clear the origin square and the square directly behind,
            // which is equivalent to a square adjacent to the origin.
            const auto& last = this->latest();
            auto& target = m_internal[last->to];
            latest.capture = {last->to, *target};
            origin = std::nullopt;
            target = std::nullopt;
        }

        else if(type == piece::move::short_castle || type == piece::move::long_castle) {
            if(type == piece::move::short_castle) {
                latest.castle = {to + 1, to - 1};
            } else {
                latest.castle = {to - 2, to + 1};
            }

            auto& rook_origin = m_internal[latest.castle->from];
            auto& rook_dest = m_internal[latest.castle->to];

            // Check that the king isn't moving through check.
            // We don't have to worry about captures here.
            rook_dest = origin;
            origin = std::nullopt;

            if(this->check(m_color)) {
                origin = rook_dest;
                rook_dest = std::nullopt;
                return false;
            }

            dest = rook_dest;
            rook_dest = rook_origin;
            rook_origin = std::nullopt;
            ++dest->moves;
            ++rook_dest->moves;
        }

        else if(type == piece::move::promotion) {
            if(dest) {
                latest.capture = {to, *dest};
            }

            dest = piece(origin->hue, piece::type::queen);
            dest->moves = origin->moves;
            origin = std::nullopt;
            latest.promotion = dest;
        }

        else {
            throw std::runtime_error("unimplemented movement type");
        }
    }

    m_history.push_back(latest);

    // If the move is actually legal, finish updating the board state and return.
    if(m_anarchy || !this->check(m_color)) {
        bool pawn = dest->variety != piece::type::pawn;
        bool pacifist = type == piece::move::normal;
        bool white = m_color == piece::color::white;
        m_trivials = (pawn && pacifist) ? m_trivials + 1 : 0;
        m_color = (white) ? piece::color::black : piece::color::white;
        return true;
    }

    this->undo();
    return false;
}

void bongcloud::board::load(const std::string_view string) {
    using piece = bongcloud::piece;
    using color = bongcloud::piece::color;
    using type = bongcloud::piece::type;

    // FEN strings start with piece placement from the A1 square.
    std::size_t square = 0;
    std::size_t character = 0;
    char c;

    // First, we handle piece placement.
    while((c = string.at(character++)) != ' ') {
        switch(c) {
            // Lowercase represent white pieces, uppercase represent black pieces.
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
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                square += c - '0';
                break;

            case '/': // A slash moves the cursor to the next rank.
                square = (((square - 1) / length) * length) + length;
                break;

            default: {
                auto comment = fmt::format("illegal FEN piece type: {}", c);
                throw std::runtime_error(comment);
            }
        }
    }

    // Next, assign the specified active color.
    switch(string.at(character++)) {
        case 'w': m_color = color::white; break;
        case 'b': m_color = color::black; break;

        default: {
            auto comment = fmt::format("illegal FEN starting color: {}", c);
            throw std::runtime_error(comment);
        }
    }

    // Next, handle castling rights.
    // Array corresponds to: white short, white long, black short, black long.
    std::array<std::optional<std::size_t>, 4> forbidden_castles = {
        0, length - 1, length * (length - 1), (length * length) - 1
    };

    // Advance the character cursor since the previous
    // switch would have left it on a space character.
    ++character;

    while((c = string.at(character++)) != ' ') {
        switch(c) {
            case 'K': forbidden_castles[0] = std::nullopt; break;
            case 'Q': forbidden_castles[1] = std::nullopt; break;
            case 'k': forbidden_castles[2] = std::nullopt; break;
            case 'q': forbidden_castles[3] = std::nullopt; break;
            case '-': break;

            default: {
                auto comment = fmt::format("illegal character encountered while parsing castling rights: {}", c);
                throw std::runtime_error(comment);
            }
        }
    }

    for(auto index : forbidden_castles) {
        if(!index) {
            continue;
        }

        auto& piece = m_internal[*index];
        if(piece && piece->variety == piece::type::rook) {
            piece->moves = 1;
        }
    }

    // Next, handle the en passant target square.
    if(string.at(character) == '-') {
        character += 2;
    } else {
        const char file = string.at(character++);
        const char rank = string.at(character++);
        std::size_t capture_square = ((rank - '1') * length) + (file - 'a');

        // TODO: Validate the capture square and make sure it isn't on a square
        // which could be impossible to capture en passant on.

        record latest;
        latest.trivials = m_trivials;

        latest.move = {
            (m_color == color::white) ? capture_square + length : capture_square - length,
            (m_color == color::white) ? capture_square - length : capture_square + length
        };

        m_history.push_back(latest);
    }

    // Advance the character cursor since the previous
    // switch would have left it on a space character.
    ++character;

    // Handle the half-move clock via string-to-integer conversion.
    // The full-move count is not handled explicitly as we have no use for it.
    std::from_chars(string.begin() + character, string.end(), m_trivials);

    fmt::print("[bongcloud] FEN string loaded, new board state:\n");
    this->print();
}

void bongcloud::board::undo(void) {
    if(m_history.size() == 0) {
        return;
    }

    const auto& last = m_history.back();
    auto& origin = m_internal[last.move.from];
    auto& dest = m_internal[last.move.to];

    // Revert the board to its previous state.
    if(last.promotion) {
        dest = piece(last.promotion->hue, piece::type::pawn);
    }

    origin = dest;
    origin->moves--;
    dest = std::nullopt;

    if(last.capture) {
        auto& capture = m_internal[last.capture->index];
        capture = last.capture->piece;
    }

    if(last.castle) {
        auto& rook_origin = m_internal[last.castle->from];
        auto& rook_dest = m_internal[last.castle->to];
        rook_origin = rook_dest;
        rook_origin->moves--;
        rook_dest = std::nullopt;
    }

    m_color = last.color;
    m_trivials = last.trivials;
    m_history.pop_back();
}
