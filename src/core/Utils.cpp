#include <tiletwister/core/Utils.hpp>

#include <algorithm>
#include <chrono>

namespace Utils {

std::mt19937& rng() {
  static std::mt19937 gen(static_cast<std::mt19937::result_type>(
      std::chrono::high_resolution_clock::now().time_since_epoch().count()));
  return gen;
}

int randInt(int loInclusive, int hiInclusive) {
  std::uniform_int_distribution<int> dist(loInclusive, hiInclusive);
  return dist(rng());
}

bool chance(int numerator, int denominator) {
  if (denominator <= 0) return false;
  return randInt(1, denominator) <= numerator;
}

float clampf(float v, float lo, float hi) {
  return std::max(lo, std::min(v, hi));
}

float lerp(float a, float b, float t) { return a + (b - a) * t; }

float easeOutCubic(float t) {
  t = clampf(t, 0.0f, 1.0f);
  const float u = 1.0f - t;
  return 1.0f - u * u * u;
}

std::vector<std::pair<int, int>> emptyCells(const int grid[4][4]) {
  std::vector<std::pair<int, int>> out;
  out.reserve(16);
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (grid[r][c] == 0) out.emplace_back(r, c);
    }
  }
  return out;
}

} // namespace Utils


