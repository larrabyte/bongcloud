#include "board.h"

bool Board::loadfen(const char* position) {
    // FEN strings start from the A1 square.
    std::size_t cursor = this->square("a1");
    std::size_t rank = cursor;
    bool invalid = false;
    bool parsed = false;

    while(!parsed && !invalid) {
        Piece &piece = this->square(cursor);
        char c = *position++;

        // Make sure that the cursor hasn't escaped the bounds of the board.
        invalid = cursor > rank + this->squares || cursor > this->elements;

        switch(c) {
            // Lowercase letters represent white pieces.
            case 'r': piece.set(Piece::Colour::white, Piece::Type::rook); cursor++; break;
            case 'n': piece.set(Piece::Colour::white, Piece::Type::knight); cursor++; break;
            case 'b': piece.set(Piece::Colour::white, Piece::Type::bishop); cursor++; break;
            case 'q': piece.set(Piece::Colour::white, Piece::Type::queen); cursor++; break;
            case 'k': piece.set(Piece::Colour::white, Piece::Type::king); cursor++; break;
            case 'p': piece.set(Piece::Colour::white, Piece::Type::pawn); cursor++; break;

            // Uppercase letters represent black pieces.
            case 'R': piece.set(Piece::Colour::black, Piece::Type::rook); cursor++; break;
            case 'N': piece.set(Piece::Colour::black, Piece::Type::knight); cursor++; break;
            case 'B': piece.set(Piece::Colour::black, Piece::Type::bishop); cursor++; break;
            case 'Q': piece.set(Piece::Colour::black, Piece::Type::queen); cursor++; break;
            case 'K': piece.set(Piece::Colour::black, Piece::Type::king); cursor++; break;
            case 'P': piece.set(Piece::Colour::black, Piece::Type::pawn); cursor++; break;

            // Numbers signify the no. of squares to skip.
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                cursor += c - '0';
                break;

            case '/': // A slash moves the cursor to the next rank.
                cursor = rank - this->squares;
                rank = cursor;
                break;

            case ' ': case '\0': // End of string, FEN string parsed.
                parsed = true;
                break;

            default: // Unknown character, abort!
                invalid = true;
                break;
        }
    }

    return parsed;
}

bool Board::islegal(std::size_t a, std::size_t b) {
    Piece& ap = this->square(a);
    Piece& bp = this->square(b);

    // Special case: we're just moving the piece back.
    if(a == b) return true;

    // Otherwise, check our standard cases.
    bool player = true; // this->player == ap.colour;
    bool other = ap.colour != bp.colour || bp.type == Piece::Type::empty;
    bool movable = false;

    switch(ap.type) {
        // Empty pieces don't move.
        case Piece::Type::empty: {
            break;
        }

        // Pawns can move two squares on their first move, otherwise one square.
        case Piece::Type::pawn: {
            if(this->player == Piece::Colour::white) {
                bool twosquare = this->onrank('2', a) ? (a - 2 * this->squares) == b : false;
                bool onesquare = a - this->squares == b;
                bool obstacle = onesquare && bp.type != Piece::Type::empty;
                bool adjacent = bp.type != Piece::Type::empty && (b == a - this->squares + 1 || b == a - this->squares - 1);
                movable = (twosquare || onesquare || adjacent) && !obstacle;
            } else if(this->player == Piece::Colour::black) {
                bool twosquare = this->onrank('7', a) ? (a + 2 * this->squares) == b : false;
                bool onesquare = a + this->squares == b;
                bool obstacle = onesquare && bp.type != Piece::Type::empty;
                bool adjacent = bp.type != Piece::Type::empty && (b == a + this->squares + 1 || b == a + this->squares - 1);
                movable = (twosquare || onesquare || adjacent) && !obstacle;
            }

            break;
        }

        // ?????
        default: {
            movable = true;
            break;
        }
    }

    return player && other && movable;
}

void Board::advance(void) {
    switch(this->player) {
        case Piece::Colour::white: this->player = Piece::Colour::black; break;
        case Piece::Colour::black: this->player = Piece::Colour::white; break;
    }
}

bool Board::onrank(const char rank, std::size_t index) {
    std::size_t rankidx = rank - '0';
    std::size_t current = 8 - (index / this->squares);
    return current == rankidx;
}

bool Board::onfile(const char file, std::size_t index) {
    std::size_t fileidx = file - 'a';
    return index % fileidx == 0;
}

std::size_t Board::square(const char* location) {
    // Find the second-last rank by multiplying board length: (k - 1) * k.
    std::size_t base = (this->squares - 1) * this->squares;

    std::size_t rank = base - (location[1] - '1') * this->squares;
    std::size_t file = location[0] - 'a';
    return rank + file;
}

Piece::Colour Board::current(void) {
    return this->player;
}

Piece& Board::square(std::size_t index) {
    return this->array[index];
}

Piece* Board::begin(void) {
    return this->array;
}

Piece* Board::end(void) {
    return this->array + this->elements;
}

Board::Board(std::size_t squares) {
    this->squares = squares;
    this->elements = squares * squares;
    this->array = new Piece[this->elements];
    this->player = Piece::Colour::white;

    for(auto& piece : *this) {
        // Ensure that each piece is set to a valid value as
        // this data is coming from the heap (segfaults galore).
        piece.set(Piece::Colour::white, Piece::Type::empty);
    }
}
