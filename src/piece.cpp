#include "piece.h"

void Piece::set(Colour colour, Type type) {
    this->colour = colour;
    this->type = type;
}

void Piece::copy(Piece& other) {
    this->set(other.colour, other.type);
}

void Piece::swap(Piece& other) {
    Piece store;
    store.set(other.colour, other.type);
    other.set(this->colour, this->type);
    this->set(store.colour, store.type);
}
