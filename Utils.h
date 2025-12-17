#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace Utils
{

    // Random
    std::mt19937 &rng();
    int randInt(int loInclusive, int hiInclusive);
    bool chance(int numerator, int denominator);

    // Math
    float clampf(float v, float lo, float hi);
    float lerp(float a, float b, float t);
    float easeOutCubic(float t);

    // Colors
    SDL_Color backgroundPink(); // RGB(255, 182, 193)
    SDL_Color tileColor(int value);
    SDL_Color tileBorderColor();
    SDL_Color gridEmptyCellColor();

    // Board helpers
    std::vector<std::pair<int, int>> emptyCells(const int grid[4][4]);

} // namespace Utils
