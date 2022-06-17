#include "pieces.hpp"
#include "extras.hpp"
#include "board.hpp"

#include <centurion.hpp>
#include <fmt/core.h>
#include <algorithm>
#include <stdexcept>
#include <charconv>
#include <cassert>
#include <cctype>

namespace detail {
    std::size_t perft(bcl::board& board, const std::size_t depth) noexcept {
        if(depth == 0) {
            return 1;
        }

        std::size_t count = 0;
        for(const auto& move : board.moves()) {
            board.move(move.from, move.to);
            count += perft(board, depth - 1);
            board.undo();
        }

        return count;
    }
}

bool bcl::board::move(const std::size_t from, const std::size_t to) noexcept {
    auto& origin = m_internal[from];
    auto& dest = m_internal[to];

    assert(origin.has_value());
    assert(from != to);

    // First, make sure that there are no trivial conditions preventing a move.
    // This could be either a forced stalemate (50 move rule), attempting to
    // move an enemy piece or attempting to capture a friendly piece.
    if(m_trivials >= constants::trivial_force_draw || origin->hue != m_color || (dest && dest->hue == m_color)) {
        return false;
    }

    if(m_anarchy) {
        // Anarchy mode is limited to normal moves and capturing moves,
        // since regular piece movement rules do not apply.
        bcl::record ffa;
        ffa.color = m_color;
        ffa.move = {from, to};
        ffa.trivials = m_trivials;

        if(dest) {
            ffa.capture = {to, *dest};
        }

        dest = origin;
        origin = std::nullopt;
        m_history.push_back(ffa);
        return true;
    }

    // Otherwise, check if the move is pseudolegal and move the pieces accordingly.
    if(auto type = this->pseudolegal(from, to)) {
        bcl::record history;
        history.color = m_color;
        history.move = {from, to};
        history.trivials = m_trivials;
        history.rights = m_rights;

        switch(*type) {
            case piece::move::normal: {
                if(origin->variety == piece::type::rook) {
                    if(from == 0 || from == length * (length - 1)) {
                        m_rights[origin->hue].kingside = false;
                    } else if(from == length - 1 || from == (length * length) - 1) {
                        m_rights[origin->hue].queenside = false;
                    }
                }

                else if(origin->variety == piece::type::king) {
                    m_rights[origin->hue].kingside = false;
                    m_rights[origin->hue].queenside = false;
                }

                break;
            }

            case piece::move::capture: {
                if(origin->variety == piece::type::rook) {
                    if(from == 0 || from == length * (length - 1)) {
                        m_rights[origin->hue].kingside = false;
                    } else if(from == length - 1 || from == (length * length) - 1) {
                        m_rights[origin->hue].queenside = false;
                    }
                }

                else if(origin->variety == piece::type::king) {
                    m_rights[origin->hue].kingside = false;
                    m_rights[origin->hue].queenside = false;
                }

                history.capture = {to, *dest};
                break;
            }

            case piece::move::en_passant: {
                const auto& latest = this->latest();
                auto& target = m_internal[latest->to];
                history.capture = {latest->to, *target};
                target = std::nullopt;
                break;
            }

            case piece::move::short_castle: {
                // Since this has already been validated and confirmed to be pseudolegal,
                // we can use the king's position as an anchor and perform arithmetic relative to it.
                history.castle = {((from / length) * length) + length - 1, to - 1};

                // If the king is already in check, we cannot castle.
                if(this->check()) {
                    return false;
                }

                // Check that the king isn't moving through check.
                // We don't have to worry about accidental captures here.
                for(std::size_t delta = 1; delta < to - from; ++delta) {
                    auto& trail = m_internal[from + delta - 1];
                    auto& cursor = m_internal[from + delta];
                    m_kings[origin->hue] = from + delta;
                    cursor = trail;
                    trail = std::nullopt;

                    if(this->check()) {
                        m_kings[origin->hue] = from;
                        origin = cursor;
                        cursor = std::nullopt;
                        return false;
                    }
                }

                // Since the king is now on the rook's destination square,
                // we move the king back to its original square before moving the rook.
                auto& rook = m_internal[history.castle->from];
                auto& king = m_internal[history.castle->to];
                m_rights[origin->hue].kingside = false;
                origin = king;
                king = rook;
                rook = std::nullopt;
                break;
            }

            case piece::move::long_castle: {
                // Since this has already been validated and confirmed to be pseudolegal,
                // we can use the king's position as an anchor and perform arithmetic relative to it.
                history.castle = {(from / length) * length, to + 1};

                // If the king is already in check, we cannot castle.
                if(this->check()) {
                    return false;
                }

                // Check that the king isn't moving through check.
                // We don't have to worry about accidental captures here.
                for(std::size_t delta = 1; delta < from - to; ++delta) {
                    auto& trail = m_internal[from - delta + 1];
                    auto& cursor = m_internal[from - delta];
                    m_kings[origin->hue] = from - delta;
                    cursor = trail;
                    trail = std::nullopt;

                    if(this->check()) {
                        m_kings[origin->hue] = from;
                        origin = cursor;
                        cursor = std::nullopt;
                        return false;
                    }
                }

                // Since the king is now on the rook's destination square,
                // we move the king to its proper destination before moving the rook.
                auto& rook = m_internal[history.castle->from];
                auto& king = m_internal[history.castle->to];
                m_rights[origin->hue].queenside = false;
                origin = king;
                king = rook;
                rook = std::nullopt;
                break;
            }

            case piece::move::promotion: {
                if(dest) {
                    history.capture = {to, *dest};
                }

                // Instead of placing a promoted piece immediately,
                // we can use the common piece movement code and just set
                // the origin square to contain the promoted piece.
                origin = {origin->hue, piece::type::queen};
                history.promotion = origin;
                break;
            }
        }

        // Every move involves the same sequence of copying the piece
        // from the origin square to the destination square and then
        // clearing the origin square and incrementing the move count.
        dest = origin;
        origin = std::nullopt;
        m_history.push_back(std::move(history));

        if(dest->variety == piece::type::king) {
            m_kings[origin->hue] = to;
        }

        // Check that the move just played did not leave the king in check.
        if(this->check()) {
            this->undo();
            return false;
        }

        // Otherwise, finalise the state of the board.
        bool trivial = dest->variety != piece::type::pawn && type == piece::move::normal;
        m_trivials = (trivial) ? m_trivials + 1 : 0;
        m_color = ext::flip(m_color);
        return true;
    };

    return false;
}

std::vector<bcl::move> bcl::board::moves(void) noexcept {
    // Preallocate space here so we don't spend time resizing and copying.
    std::vector<bcl::move> moves;
    moves.reserve(constants::move_buffer_reserve);

    for(std::size_t from = 0; from < length * length; ++from) {
        for(std::size_t to = 0; to < length * length; ++to) {
            if(from != to && m_internal[from] && this->move(from, to)) {
                bcl::move m = {from, to};
                moves.push_back(m);
                this->undo();
            }
        }
    }

    return moves;
}

std::size_t bcl::board::positions(const std::size_t depth) noexcept {
    return detail::perft(*this, depth);
}

bool bcl::board::check(void) const noexcept {
    std::size_t king = m_kings[m_color];

    for(std::size_t i = 0; i < length * length; ++i) {
        const auto& piece = m_internal[i];
        if(piece && piece->hue != m_color && this->pseudolegal(i, king)) {
            return true;
        }
    }

    return false;
}

bool bcl::board::checkmate(void) noexcept {
    return this->check() && this->moves().empty();
}

bool bcl::board::stalemate(void) noexcept {
    return this->moves().empty() && !this->check();
}

void bcl::board::print(void) const noexcept {
    // Start from the top-left square.
    std::size_t rank = length - 1;
    std::size_t file = 0;
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

            if(c != '?' && piece->hue == piece::color::white) {
                // Casting is OK since all possible values of c are valid char values.
                c = static_cast<char>(std::toupper(c));
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

void bcl::board::load(const std::string_view string) {
    using color = bcl::piece::color;
    using type = bcl::piece::type;

    // FEN strings start with piece placement from the top-left square.
    std::size_t character = 0;
    std::size_t rank = length - 1;
    std::size_t file = 0;
    char c;

    // First, we handle piece placement.
    pair<bool> royals = {false, false};

    while((c = string.at(character++)) != ' ') {
        std::size_t square = (rank * length) + file;

        switch(c) {
            // Uppercase represents white pieces, lowercase represents black pieces.
            case 'R': m_internal[square] = {color::white, type::rook}; break;
            case 'N': m_internal[square] = {color::white, type::knight}; break;
            case 'B': m_internal[square] = {color::white, type::bishop}; break;
            case 'Q': m_internal[square] = {color::white, type::queen}; break;

            case 'K': {
                if(royals[color::white]) {
                    throw std::runtime_error("multiple kings are forbidden");
                }

                m_internal[square] = {color::white, type::king};
                m_kings[color::white] = square;
                royals[color::white] = true;
                break;
            }

            case 'P': m_internal[square] = {color::white, type::pawn}; break;
            case 'r': m_internal[square] = {color::black, type::rook}; break;
            case 'n': m_internal[square] = {color::black, type::knight}; break;
            case 'b': m_internal[square] = {color::black, type::bishop}; break;
            case 'q': m_internal[square] = {color::black, type::queen}; break;

            case 'k': {
                if(royals[color::black]) {
                    throw std::runtime_error("multiple kings are forbidden");
                }

                m_internal[square] = {color::black, type::king};
                m_kings[color::black] = square;
                royals[color::black] = true;
                break;
            }

            case 'p': m_internal[square] = {color::black, type::pawn}; break;

            // Numbers signify the number of squares to skip.
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                file += static_cast<std::size_t>(c - '1');
                break;

            case '/': // A slash moves the cursor to the next rank.
                file = length;
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
    switch((c = string.at(character++))) {
        case 'w': m_color = color::white; break;
        case 'b': m_color = color::black; break;

        default: {
            auto comment = fmt::format("illegal FEN starting color: {}", c);
            throw std::runtime_error(comment);
        }
    }

    // Advance the character cursor since the previous
    // switch would have left it on a space character.
    ++character;

    // Next, handle castling rights.
    m_rights = {rights {false, false}, rights {false, false}};

    while((c = string.at(character++)) != ' ') {
        switch(c) {
            case 'K': m_rights[color::white].kingside = true; break;
            case 'Q': m_rights[color::white].queenside = true; break;
            case 'k': m_rights[color::black].kingside = true; break;
            case 'q': m_rights[color::black].queenside = true; break;
            case '-': break;

            default: {
                auto comment = fmt::format("illegal character encountered while parsing castling rights: {}", c);
                throw std::runtime_error(comment);
            }
        }
    }

    // Next, handle the en passant target square.
    if(string.at(character) == '-') {
        character += 2;
    } else {
        char file_char = string.at(character++);
        char rank_char = string.at(character++);
        auto file_number = static_cast<std::size_t>(file_char - 'a');
        auto rank_number = static_cast<std::size_t>(rank_char - '1');
        auto capture_square = (rank_number * length) + file_number;

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

    // Handle the half-move clock via string-to-integer conversion.
    // The full-move count is not handled explicitly as we have no use for it.
    std::from_chars(string.begin() + character, string.end(), m_trivials);
}

void bcl::board::undo(void) noexcept {
    assert(!m_history.empty());

    const auto& last = m_history.back();
    auto& origin = m_internal[last.move.from];
    auto& dest = m_internal[last.move.to];

    // If the move was a promotion, then we don't care about what's on the destination square.
    origin = (last.promotion) ? piece {last.promotion->hue, piece::type::pawn} : dest;
    dest = std::nullopt;

    if(origin->variety == piece::type::king) {
        m_kings[origin->hue] = last.move.from;
    }

    if(last.capture) {
        // This is separate because of the possibility of en-passant.
        auto& capture = m_internal[last.capture->index];
        capture = last.capture->piece;
    }

    if(last.castle) {
        auto& source = m_internal[last.castle->from];
        auto& sink = m_internal[last.castle->to];
        source = sink;
        sink = std::nullopt;
    }

    m_rights = last.rights;
    m_color = last.color;
    m_trivials = last.trivials;
    m_history.pop_back();
}
