#include "board.h"

void Piece::set(Colour colour, Type type) {
    this->colour = colour;
    this->type = type;
}

bool Board::loadfen(const char *location) {
    // FEN strings start from the A1 square.
    std::size_t cursor = this->square("a1");
    std::size_t rank = cursor;

    // If the string is empty, parsing is "done".
    bool parsed = *location == '\0' ? true : false;

    for(char c = *location; c != '\0'; c = *++location) {
        Piece &piece = this->square(cursor);
        bool invalid = cursor > rank + this->squares || cursor > this->elements;

        // Break out if we're done or something went wrong.
        if(parsed || invalid) break;

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

            // Numbers represent squares to skip.
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                cursor += c - '0';
                break;

            case '/': // A slash moves the cursor to the next rank.
                cursor = rank - this->squares;
                rank = cursor;
                break;

            // Unknown character, done.
            default: parsed = true; break;
        }
    }

    // To ensure all control-paths are covered, we break out of the for-loop
    // with the flag we wish to return set in parsed.
    return parsed;
}

std::size_t Board::square(const char *location) {
    // Find the second-last rank by multiplying board length: (k - 1) * k.
    std::size_t base = (this->squares - 1) * this->squares;

    std::size_t rank = base - (location[1] - '1') * this->squares;
    std::size_t file = (location[0] - 'a');
    return rank + file;
}

Piece &Board::square(std::size_t index) {
    return this->array[index];
}

Piece *Board::begin(void) {
    return this->array;
}

Piece *Board::end(void) {
    return this->array + this->elements;
}

Board::Board(std::size_t squares) {
    this->elements = squares * squares;
    this->array = new Piece[this->elements];
    this->squares = squares;

    for(auto &piece : *this) {
        // Ensure that each piece is set to a valid value as
        // this data is coming from the heap (segfaults galore).
        piece.set(Piece::Colour::white, Piece::Type::empty);
    }
}
