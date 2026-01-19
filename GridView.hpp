#pragma once
#include <SDL.h>

class GridView {
public:
    GridView() = default;
    ~GridView() = default;

    SDL_Rect boardRect(int windowW, int windowH) const;
    SDL_Rect cellRect(int windowW, int windowH, float row, float col) const;
    void fillRoundRect(SDL_Renderer* r, const SDL_Rect& rect, int radius,
                       SDL_Color color) const;
};
