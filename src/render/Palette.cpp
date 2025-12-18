#include <tiletwister/render/Palette.hpp>

#include <tiletwister/core/Utils.hpp>

namespace {

static SDL_Color mix(SDL_Color a, SDL_Color b, float t) {
  t = Utils::clampf(t, 0.0f, 1.0f);
  return SDL_Color{
      static_cast<Uint8>(Utils::lerp(a.r, b.r, t)),
      static_cast<Uint8>(Utils::lerp(a.g, b.g, t)),
      static_cast<Uint8>(Utils::lerp(a.b, b.b, t)),
      255,
  };
}

} // namespace

namespace Palette {

SDL_Color backgroundPink() { return SDL_Color{255, 182, 193, 255}; }

SDL_Color tileBorderColor() { return SDL_Color{255, 255, 255, 70}; }

SDL_Color gridEmptyCellColor() { return SDL_Color{255, 255, 255, 45}; }

SDL_Color tileColor(int value) {
  // Pleasant palette that gets more saturated as the value increases.
  // Values beyond 2048 clamp to the strongest color.
  struct Stop {
    int v;
    SDL_Color c;
  };
  const Stop stops[] = {
      {0, SDL_Color{0, 0, 0, 0}},
      {2, SDL_Color{255, 245, 250, 255}},
      {4, SDL_Color{255, 230, 245, 255}},
      {8, SDL_Color{255, 205, 235, 255}},
      {16, SDL_Color{255, 175, 225, 255}},
      {32, SDL_Color{255, 145, 215, 255}},
      {64, SDL_Color{255, 110, 205, 255}},
      {128, SDL_Color{255, 80, 190, 255}},
      {256, SDL_Color{250, 55, 170, 255}},
      {512, SDL_Color{240, 35, 150, 255}},
      {1024, SDL_Color{225, 20, 125, 255}},
      {2048, SDL_Color{205, 10, 105, 255}},
  };

  if (value <= 0) return SDL_Color{0, 0, 0, 0};
  if (value >= 2048) return stops[sizeof(stops) / sizeof(stops[0]) - 1].c;

  for (size_t i = 1; i < sizeof(stops) / sizeof(stops[0]); ++i) {
    if (value <= stops[i].v) {
      const Stop& prev = stops[i - 1];
      const Stop& next = stops[i];
      const float t = static_cast<float>(value - prev.v) /
                      static_cast<float>(next.v - prev.v);
      return mix(prev.c, next.c, t);
    }
  }
  return stops[sizeof(stops) / sizeof(stops[0]) - 1].c;
}

} // namespace Palette


