#ifndef TILE_H
#define TILE_H

class Tile {
private:
    int value;   // 0 = vide

public:
    Tile(int v = 0);

    int getValue() const;
    void setValue(int v);

    bool isEmpty() const;
};

#endif
