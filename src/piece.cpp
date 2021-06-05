#include "piece.h"

void Piece::set(Colour colour, Type type, std::size_t movecnt) {
    this->colour = colour;
    this->type = type;
    this->movecnt = movecnt;
}

void Piece::clear(void) {
    this->set(Colour::white, Type::empty, 0);
}

void Piece::copy(Piece& other) {
    this->set(other.colour, other.type, other.movecnt);
}

void Piece::swap(Piece& other) {
    Piece store;
    store.set(other.colour, other.type, other.movecnt);
    other.set(this->colour, this->type, this->movecnt);
    this->set(store.colour, store.type, store.movecnt);
}
