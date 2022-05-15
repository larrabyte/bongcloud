#include "pieces.hpp"
#include "extras.hpp"
#include "board.hpp"

#include <centurion.hpp>
#include <fmt/core.h>
#include <cstddef>

namespace internal {
    constexpr bongcloud::piece::color color_array[] = {
        bongcloud::piece::color::black,
        bongcloud::piece::color::white
    };

    inline bongcloud::piece::color next_color(const bongcloud::piece::color color) {
        return color_array[ext::to_underlying(color)];
    }

    inline bongcloud::piece::color prev_color(const bongcloud::piece::color color) {
        return color_array[ext::to_underlying(color)];
    }
}

bongcloud::board::board(const std::size_t l, const bool a) :
    length {l},
    m_internal {l * l},
    m_anarchy {a} {

    fmt::print("[bongcloud] initialising board of size {}x{}... (anarchy: {})\n", l, l, a);
}

bongcloud::board bongcloud::board::duplicate(void) const {
    bongcloud::board board(length, m_anarchy);
    board.m_internal = m_internal;
    board.m_trivial_half_moves = m_trivial_half_moves;
    board.m_color = m_color;
    return board;
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

    fmt::print("\n");
}

bool bongcloud::board::check(const piece::color color) const {
    // Attempt to find the index of the specified player's king.
    std::vector<std::size_t> kings;

    for(std::size_t i = 0; i < length * length; i++) {
        const auto& piece = m_internal[i];
        if(piece && piece->hue == color && piece->variety == piece::type::king) {
            kings.push_back(i);
            break;
        }
    }

    if(kings.empty()) {
        throw std::runtime_error("no king(s) on board");
    }

    // Return true if any king is in check.
    for(const auto king : kings) {
        for(std::size_t i = 0; i < length * length; i++) {
            const auto& piece = m_internal[i];
            if(piece && piece->hue != color && permissible(i, king)) {
                return true;
            }
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

    if(m_trivial_half_moves == 100) {
        // Forced statemate.
        return false;
    }

    mutation latest;
    latest.trivials = m_trivial_half_moves;
    latest.move = {from, to};

    bool correct_color = origin->hue == m_color;
    bool cannibal = dest && origin->hue == dest->hue;
    auto type = permissible(from, to);
    bool pseudolegal = correct_color && !cannibal && type;

    if(m_anarchy) {
        // Since most moves are usually considered impossible, we require
        // different logic for piece movement when anarchy mode is enabled.
        if(dest) {
            latest.capture = {to, *dest};
        }

        dest = origin;
        origin = std::nullopt;
        dest->moves++;
    }

    else if(pseudolegal) {
        if(type == piece::move::normal) {
            // Move the piece forward and
            // clear the origin square.
            dest = origin;
            origin = std::nullopt;
            dest->moves++;
        }

        else if(type == piece::move::capture) {
            // Store a copy of the captured piece
            // in the mutation object before erasing.
            latest.capture = {to, *dest};

            dest = origin;
            origin = std::nullopt;
            dest->moves++;
        }

        else if(type == piece::move::en_passant) {
            // Move the piece at the origin square to the
            // destination square and update its move count.
            dest = origin;
            dest->moves++;

            // Clear the origin square and the square directly behind,
            // which is equivalent to a square adjacent to the origin.
            const auto& last = this->latest();
            auto& target = m_internal[last->to];
            latest.capture = {last->to, *target};
            origin = std::nullopt;
            target = std::nullopt;
        }

        else if(type == piece::move::short_castle || type == piece::move::long_castle) {
            // Calculate the appropriate squares to send the king and rook.
            latest.castle = {
                (type == piece::move::short_castle) ? to + 1 : to - 2,
                (type == piece::move::short_castle) ? to - 1 : to + 1
            };

            auto& rook_origin = m_internal[latest.castle->from];
            auto& rook_dest = m_internal[latest.castle->to];

            // Check that the king isn't moving through check.
            // We don't have to worry about captures here.
            rook_dest = origin;
            origin = std::nullopt;

            if(check(m_color)) {
                origin = rook_dest;
                rook_dest = std::nullopt;
                return false;
            }

            // Update piece positions and increment the move count.
            dest = rook_dest;
            rook_dest = rook_origin;
            dest->moves++;
            rook_dest->moves++;

            // Remove the original king and rook from the board.
            rook_origin = std::nullopt;
        }

        else if(type == piece::move::promotion) {
            // If there is a piece present, make sure to add a capture entry.
            if(dest) {
                latest.capture = {to, *dest};
            }

            // Create a new queen with the same move count.
            dest = piece(origin->hue, piece::type::queen);
            dest->moves = origin->moves;
            origin = std::nullopt;

            // Add the promotion entry to the mutation object.
            latest.promotion = dest;
        }

        else {
            throw std::runtime_error("unimplemented movement type");
        }
    }

    else {
        // If the move is not pseudolegal and we aren't in anarchy mode,
        // then this move must be illegal and we can simply return false.
        return false;
    }

    m_color = internal::next_color(m_color);
    m_history.push_back(std::move(latest));
    auto previous_color = internal::prev_color(m_color);

    if(m_anarchy || !check(previous_color)) {
        bool trivial = dest->variety != piece::type::pawn && type == piece::move::normal;
        m_trivial_half_moves = (trivial) ? m_trivial_half_moves + 1 : 0;
        return true;
    }

    undo();
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
    character++;

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

        mutation latest;
        latest.trivials = m_trivial_half_moves;

        latest.move = {
            (m_color == color::white) ? capture_square + length : capture_square - length,
            (m_color == color::white) ? capture_square - length : capture_square + length
        };

        m_history.push_back(latest);
    }

    // Advance the character cursor since the previous
    // switch would have left it on a space character.
    character++;

    // Handle the half-move clock via string-to-integer conversion.
    // The full-move count is not handled explicitly as we have no use for it.
    std::from_chars(string.begin() + character, string.end(), m_trivial_half_moves);

    fmt::print("[bongcloud] board.load():\n");
    this->print();
}

void bongcloud::board::undo(void) {
    // Ensure that there is a move to undo.
    if(m_history.size() == 0) {
        return;
    }

    // Retrieve the last move and undo it.
    const auto& last = m_history.back();
    auto& origin = m_internal[last.move.from];
    auto& dest = m_internal[last.move.to];

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

    // Revert to the previous player and delete the last move.
    m_color = internal::prev_color(m_color);
    m_trivial_half_moves = last.trivials;
    m_history.pop_back();
}
