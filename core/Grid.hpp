#ifndef GRID_H
#define GRID_H

#include <vector>
#include "Tile.hpp"

class Grid {
private:
    static const int SIZE = 4;
    std::vector<std::vector<Tile>> cells;

    public:
    Grid();

    void initialize();
    void print() const;
};

#endif // GRID_H