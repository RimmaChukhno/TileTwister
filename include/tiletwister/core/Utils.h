#pragma once

#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace Utils {

// Random
std::mt19937& rng();
int randInt(int loInclusive, int hiInclusive);
bool chance(int numerator, int denominator);

// Math
float clampf(float v, float lo, float hi);
float lerp(float a, float b, float t);
float easeOutCubic(float t);

// Board helpers (4x4 2048 grid)
std::vector<std::pair<int, int>> emptyCells(const int grid[4][4]);

} // namespace Utils


