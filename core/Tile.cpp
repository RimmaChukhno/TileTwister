#include "Tile.hpp"

Tile::Tile(int v) : value(v) {}

int Tile::getValue() const {
    return value;
}

void Tile::setValue(int v) {
    value = v;
}

bool Tile::isEmpty() const {
    return value == 0;
}
