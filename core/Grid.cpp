#include "Grid.hpp"
#include <iostream>

Grid::Grid() {
    initialize();
}

void Grid::initialize() {
    cells.clear();
    cells.resize(SIZE, std::vector<Tile>(SIZE));
}

void Grid::print() const {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            std::cout << cells[i][j].getValue() << "\t";
        }
        std::cout << std::endl;
    }
}
