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
            --rank;
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
    assert(from != to);

    // First, make sure that there are no trivial conditions preventing a move.
    // This could be either a forced stalement (50 move rule), moving an enemy piece
    // or attempting to capture a friendly piece.
    if(m_trivials >= 100 || origin->hue != m_color || (dest && dest->hue == m_color)) {
        return false;
    }

    if(m_anarchy) {
        // Anarchy mode is limited to normal moves and capturing moves,
        // since regular piece movement rules do not apply.
        bongcloud::record ffa;
        ffa.color = m_color;
        ffa.move = {from, to};
        ffa.trivials = m_trivials;

        if(dest) {
            ffa.capture = {to, *dest};
        }

        dest = origin;
        origin = std::nullopt;
        ++dest->moves;

        m_history.push_back(ffa);
        return true;
    }

    // Otherwise, check if the move is pseudolegal and move the pieces accordingly.
    if(auto type = this->pseudolegal(from, to)) {
        bongcloud::record history;
        history.color = m_color;
        history.move = {from, to};
        history.trivials = m_trivials;

        if(type == piece::move::capture) {
            history.capture = {to, *dest};
        }

        else if(type == piece::move::en_passant) {
            const auto& latest = this->latest();
            auto& target = m_internal[latest->to];
            history.capture = {latest->to, *target};
            target = std::nullopt;
        }

        else if(type == piece::move::short_castle) {
            // Since this has already been validated and confirmed to be pseudolegal,
            // we can use the king's position as an anchor and perform arithmetic relative to it.
            history.castle = {((from / length) * length) + length - 1, to - 1};

            // If the king is already in check, we cannot castle.
            if(this->check(m_color)) {
                return false;
            }

            // Check that the king isn't moving through check.
            // We don't have to worry about accidental captures here.
            for(std::size_t delta = 1; delta < to - from; ++delta) {
                auto& trail = m_internal[from + delta - 1];
                auto& cursor = m_internal[from + delta];
                cursor = trail;
                trail = std::nullopt;

                if(this->check(m_color)) {
                    origin = cursor;
                    cursor = std::nullopt;
                    return false;
                }
            }

            // Since the king is now on the rook's destination square,
            // we move the king back to its original square before moving the rook.
            auto& rook = m_internal[history.castle->from];
            auto& king = m_internal[history.castle->to];
            origin = king;
            king = rook;
            ++king->moves;
            rook = std::nullopt;
        }

        else if(type == piece::move::long_castle) {
            // Since this has already been validated and confirmed to be pseudolegal,
            // we can use the king's position as an anchor and perform arithmetic relative to it.
            history.castle = {(from / length) * length, to + 1};

            // If the king is already in check, we cannot castle.
            if(this->check(m_color)) {
                return false;
            }

            // Check that the king isn't moving through check.
            // We don't have to worry about accidental captures here.
            for(std::size_t delta = 1; delta < from - to; ++delta) {
                auto& trail = m_internal[from - delta + 1];
                auto& cursor = m_internal[from - delta];
                cursor = trail;
                trail = std::nullopt;

                if(this->check(m_color)) {
                    origin = cursor;
                    cursor = std::nullopt;
                    return false;
                }
            }

            // Since the king is now on the rook's destination square,
            // we move the king to its proper destination before moving the rook.
            auto& rook = m_internal[history.castle->from];
            auto& king = m_internal[history.castle->to];
            origin = king;
            king = rook;
            ++king->moves;
            rook = std::nullopt;
        }

        else if(type == piece::move::promotion) {
            if(dest) {
                history.capture = {to, *dest};
            }

            // Instead of placing a promoted piece immediately,
            // we can use the common piece movement code and just set
            // the origin square to contain the promoted piece.
            std::size_t moves = origin->moves;
            origin = piece(origin->hue, piece::type::queen);
            origin->moves = moves;
            history.promotion = origin;
        }

        // Every move involves the same sequence of copying the piece
        // from the origin square to the destination square and then
        // clearing the origin square and incrementing the move count.
        dest = origin;
        origin = std::nullopt;
        ++dest->moves;
        m_history.push_back(history);

        // Check that the move just played did not leave the king in check.
        if(this->check(m_color)) {
            this->undo();
            return false;
        }

        // Otherwise, finalise the state of the board.
        bool trivial = dest->variety != piece::type::pawn && type == piece::move::normal;
        bool white = m_color == piece::color::white;
        m_trivials = (trivial) ? m_trivials + 1 : 0;
        m_color = (white) ? piece::color::black : piece::color::white;
        return true;
    };

    return false;
}

void bongcloud::board::load(const std::string_view string) {
    using piece = bongcloud::piece;
    using color = bongcloud::piece::color;
    using type = bongcloud::piece::type;

    // FEN strings start with piece placement from the top-left square.
    std::size_t character = 0;
    std::size_t rank = length - 1;
    std::size_t file = 0;
    char c;

    // First, we handle piece placement.
    while((c = string.at(character++)) != ' ') {
        std::size_t square = (rank * length) + file;

        switch(c) {
            // Uppercase represents white pieces, lowercase represents black pieces.
            case 'R': m_internal[square] = piece(color::white, type::rook); break;
            case 'N': m_internal[square] = piece(color::white, type::knight); break;
            case 'B': m_internal[square] = piece(color::white, type::bishop); break;
            case 'Q': m_internal[square] = piece(color::white, type::queen); break;
            case 'K': m_internal[square] = piece(color::white, type::king); break;
            case 'P': m_internal[square] = piece(color::white, type::pawn); break;
            case 'r': m_internal[square] = piece(color::black, type::rook); break;
            case 'n': m_internal[square] = piece(color::black, type::knight); break;
            case 'b': m_internal[square] = piece(color::black, type::bishop); break;
            case 'q': m_internal[square] = piece(color::black, type::queen); break;
            case 'k': m_internal[square] = piece(color::black, type::king); break;
            case 'p': m_internal[square] = piece(color::black, type::pawn); break;

            // Numbers signify the number of squares to skip.
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                file += c - '1';
                break;

            case '/': // A slash moves the cursor to the next rank.
                file = 8;
                break;

            default: {
                auto comment = fmt::format("illegal FEN piece type: {}", c);
                throw std::runtime_error(comment);
            }
        }

        // If we've reached the end of the rank, move to the next one.
        if(file++ == length) {
            file = 0;
            --rank;
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
    assert(m_history.size() != 0);

    const auto& last = m_history.back();
    auto& origin = m_internal[last.move.from];
    auto& dest = m_internal[last.move.to];

    // If the move was a promotion, then we don't care about what's on the destination square.
    origin = (last.promotion) ? piece(last.promotion->hue, piece::type::pawn) : dest;
    --origin->moves;
    dest = std::nullopt;

    if(last.capture) {
        // This is separate because of the possibility of en-passant.
        auto& capture = m_internal[last.capture->index];
        capture = last.capture->piece;
    }

    if(last.castle) {
        auto& source = m_internal[last.castle->from];
        auto& sink = m_internal[last.castle->to];
        source = sink;
        --source->moves;
        sink = std::nullopt;
    }

    m_color = last.color;
    m_trivials = last.trivials;
    m_history.pop_back();
}
