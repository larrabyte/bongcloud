#include "board.h"

bool Board::onrank(std::size_t rank, std::size_t index) {
    return this->squares - (index / this->squares) == rank;
}

bool Board::onfile(std::size_t file, std::size_t index) {
    return index % file == 0;
}

std::size_t Board::square(Direction direction, std::size_t origin, std::size_t distance) {
    // Suppress the uninitialised variable warning by setting a value.
    std::size_t dest = origin;

    if(direction == Direction::north) {
        if(this->player == Piece::Colour::white) {
            dest = origin - (this->squares * distance);
        } else if(this->player == Piece::Colour::black) {
            dest = origin + (this->squares * distance);
        }
    }

    else if(direction == Direction::northeast) {
        if(this->player == Piece::Colour::white) {
            dest = origin - (this->squares * distance) + distance;
        } else if(this->player == Piece::Colour::black) {
            dest = origin + (this->squares * distance) - distance;
        }
    }

    else if(direction == Direction::east) {
        dest = origin + distance;
    }

    else if(direction == Direction::southeast) {
        if(this->player == Piece::Colour::white) {
            dest = origin + (this->squares * distance) + distance;
        } else if(this->player == Piece::Colour::black) {
            dest = origin - (this->squares * distance) - distance;
        }
    }

    else if(direction == Direction::south) {
        if(this->player == Piece::Colour::white) {
            dest = origin + (this->squares * distance);
        } else if(this->player == Piece::Colour::black) {
            dest = origin - (this->squares * distance);
        }
    }

    else if(direction == Direction::southwest) {
        if(this->player == Piece::Colour::white) {
            dest = origin + (this->squares * distance) - distance;
        } else if(this->player == Piece::Colour::black) {
            dest = origin - (this->squares * distance) + distance;
        }
    }

    else if(direction == Direction::west) {
        dest = origin - distance;
    }

    else if(direction == Direction::northwest) {
        if(this->player == Piece::Colour::white) {
            dest = origin - (this->squares * distance) - distance;
        } else if(this->player == Piece::Colour::black) {
            dest = origin + (this->squares * distance) + distance;
        }
    }

    return dest;
}

void Board::advance(Piece& a, Piece& b, std::size_t prev) {
    // Overwrite piece data (a replaces b).
    b.copy(a);
    a.clear();

    // Set the board's previous move and update piece move count.
    this->lastmove = prev;
    b.movecnt++;

    // Advance to the next colour.
    switch(this->player) {
        case Piece::Colour::white: this->player = Piece::Colour::black; break;
        case Piece::Colour::black: this->player = Piece::Colour::white; break;
        default: break;
    }
}

bool Board::loadfen(const char* string) {
    // FEN strings start from the A1 square.
    std::size_t cursor = this->square("a1");
    std::size_t rank = cursor;
    bool invalid = false;
    bool pieces = false;

    // Start placing pieces on the board.
    while(!pieces && !invalid) {
        Piece& piece = this->square(cursor);
        invalid = cursor > rank + this->squares || cursor > this->elements;

        switch(*string++) {
            // Lowercase letters represent white pieces, uppercase represent black pieces.
            case 'r': piece.set(Piece::Colour::white, Piece::Type::rook, 0); cursor++; break;
            case 'n': piece.set(Piece::Colour::white, Piece::Type::knight, 0); cursor++; break;
            case 'b': piece.set(Piece::Colour::white, Piece::Type::bishop, 0); cursor++; break;
            case 'q': piece.set(Piece::Colour::white, Piece::Type::queen, 0); cursor++; break;
            case 'k': piece.set(Piece::Colour::white, Piece::Type::king, 0); cursor++; break;
            case 'p': piece.set(Piece::Colour::white, Piece::Type::pawn, 0); cursor++; break;
            case 'R': piece.set(Piece::Colour::black, Piece::Type::rook, 0); cursor++; break;
            case 'N': piece.set(Piece::Colour::black, Piece::Type::knight, 0); cursor++; break;
            case 'B': piece.set(Piece::Colour::black, Piece::Type::bishop, 0); cursor++; break;
            case 'Q': piece.set(Piece::Colour::black, Piece::Type::queen, 0); cursor++; break;
            case 'K': piece.set(Piece::Colour::black, Piece::Type::king, 0); cursor++; break;
            case 'P': piece.set(Piece::Colour::black, Piece::Type::pawn, 0); cursor++; break;

            // Numbers signify the no. of squares to skip.
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                cursor += *string - '0';
                break;

            case '/': // A slash moves the cursor to the next rank.
                cursor = rank - this->squares;
                rank = cursor;
                break;

            case ' ': // All pieces have been placed.
                pieces = true;
                break;

            default: // Unknown character, abort!
                invalid = true;
                break;
        }
    }

    // Whose turn is it to move?
    switch(*string) {
        case 'w': this->player = Piece::Colour::white; break;
        case 'b': this->player = Piece::Colour::black; break;
        default: invalid = true; break;
    }

    return pieces && !invalid;
}

bool Board::move(std::size_t a, std::size_t b) {
    Piece& ap = this->square(a);
    Piece& bp = this->square(b);

    // Special case: we're just moving the piece back.
    if(a == b) return true;

    // Otherwise, check our standard cases.
    bool player = this->player == ap.colour;
    bool other = ap.colour != bp.colour || bp.type == Piece::Type::empty;
    bool movable = false;

    switch(ap.type) {
        // Empty pieces don't move.
        case Piece::Type::empty: {
            break;
        }

        // Pawns can move two squares on their first move, otherwise one square.
        case Piece::Type::pawn: {
            bool doubles = ap.movecnt == 0 && b == this->square(Direction::north, a, 2);
            bool single = b == this->square(Direction::north, a, 1);
            bool clear = bp.type == Piece::Type::empty;
            bool standard = (doubles || single) && clear;

            std::size_t frontl = this->square(Direction::northwest, a, 1);
            std::size_t frontr = this->square(Direction::northeast, a, 1);
            bool infront = b == frontl || b == frontr;
            bool pieceahead = infront && bp.type != Piece::Type::empty;

            // EN PASSANT HON HON
            Piece& holyhell = this->square(this->square(Direction::south, b, 1));
            bool previous = this->lastmove == this->square(Direction::north, b, 1);
            bool passant = infront && previous && holyhell.type == Piece::Type::pawn;

            // Because en passant is special and is the only capture where the piece
            // does not end up on the capture square, we perform the capture here.
            if(passant) holyhell.clear();

            movable = standard || pieceahead || passant;
            break;
        }

        // ?????
        default: {
            movable = true;
            break;
        }
    }

    // If the move is possible, replace pieces.
    bool success = player && other && movable;
    if(success) this->advance(ap, bp, a);
    return success;
}

std::size_t Board::square(const char* location) {
    // Find the second-last rank by multiplying board length: (k - 1) * k.
    std::size_t base = (this->squares - 1) * this->squares;

    std::size_t rank = base - (location[1] - '1') * this->squares;
    std::size_t file = location[0] - 'a';
    return rank + file;
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

    // Set the initial last move to an impossible index.
    this->lastmove = this->elements + 1;

    for(auto& piece : *this) {
        // Ensure that each piece is set to a valid value as
        // this data is coming from the heap (segfaults galore).
        piece.clear();
    }
}
