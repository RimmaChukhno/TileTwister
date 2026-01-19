#pragma once
#include <SDL.h>

class TileView {
public:
    TileView() = default;
    ~TileView() = default;

    void drawNumber(SDL_Renderer* r, const SDL_Rect& rect, int value) const;

private:
    void drawDigit(SDL_Renderer* r, int digit, int x, int y, int w, int h, SDL_Color color) const;
};
