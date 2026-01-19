#include "Renderer.hpp"

#include "Game.hpp"
#include "Tile.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace {

SDL_Color textColorFor(int value) {
  // Dark text on light tiles, white text on strong tiles.
  if (value <= 8) return SDL_Color{80, 40, 60, 255};
  return SDL_Color{255, 255, 255, 235};
}

void setColor(SDL_Renderer* r, SDL_Color c) {
  SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

} // namespace

SDL_Rect Renderer::boardRect(int windowW, int windowH) const {
  const int size = std::min(windowW, windowH) - 80;
  const int x = (windowW - size) / 2;
  const int y = (windowH - size) / 2;
  return SDL_Rect{x, y, size, size};
}

SDL_Rect Renderer::cellRect(int windowW, int windowH, float row,
                            float col) const {
  const SDL_Rect b = boardRect(windowW, windowH);
  const int gap = 12;
  const int cell = (b.w - gap * 5) / 4;

  const float px = static_cast<float>(b.x + gap) +
                   col * static_cast<float>(cell + gap);
  const float py = static_cast<float>(b.y + gap) +
                   row * static_cast<float>(cell + gap);

  return SDL_Rect{static_cast<int>(std::round(px)),
                  static_cast<int>(std::round(py)), cell, cell};
}

void Renderer::fillRoundRect(SDL_Renderer* r, const SDL_Rect& rect, int radius,
                             SDL_Color color) const {
  // Simple approximation: fill rect + 4 corner circles.
  // (Good enough for this project, keeps dependencies minimal.)
  setColor(r, color);
  SDL_RenderFillRect(r, &rect);

  // Cut corners by overdrawing background would require stencil; instead draw
  // a softer look via border & slight alpha. Keep it simple here.
  (void)radius;
}

void Renderer::drawDigit(SDL_Renderer* r, int digit, int x, int y, int w, int h,
                         SDL_Color color) const {
  // Seven segments: a b c d e f g
  //   a
  // f   b
  //   g
  // e   c
  //   d
  static const int segs[10] = {
      /*0*/ 0b1111110,
      /*1*/ 0b0110000,
      /*2*/ 0b1101101,
      /*3*/ 0b1111001,
      /*4*/ 0b0110011,
      /*5*/ 0b1011011,
      /*6*/ 0b1011111,
      /*7*/ 0b1110000,
      /*8*/ 0b1111111,
      /*9*/ 0b1111011,
  };
  const int mask = (digit >= 0 && digit <= 9) ? segs[digit] : 0;
  setColor(r, color);

  const int t = std::max(2, w / 6); // segment thickness
  const int pad = std::max(2, t / 2);

  auto segRect = [&](char seg) -> SDL_Rect {
    switch (seg) {
    case 'a':
      return SDL_Rect{x + pad, y, w - 2 * pad, t};
    case 'd':
      return SDL_Rect{x + pad, y + h - t, w - 2 * pad, t};
    case 'g':
      return SDL_Rect{x + pad, y + (h - t) / 2, w - 2 * pad, t};
    case 'f':
      return SDL_Rect{x, y + pad, t, (h - 3 * pad) / 2};
    case 'b':
      return SDL_Rect{x + w - t, y + pad, t, (h - 3 * pad) / 2};
    case 'e':
      return SDL_Rect{x, y + (h + pad) / 2, t, (h - 3 * pad) / 2};
    case 'c':
      return SDL_Rect{x + w - t, y + (h + pad) / 2, t, (h - 3 * pad) / 2};
    default:
      return SDL_Rect{x, y, 0, 0};
    }
  };

  auto drawIf = [&](char seg, int bit) {
    if (mask & bit) {
      SDL_Rect rr = segRect(seg);
      SDL_RenderFillRect(r, &rr);
    }
  };

  drawIf('a', 1 << 6);
  drawIf('b', 1 << 5);
  drawIf('c', 1 << 4);
  drawIf('d', 1 << 3);
  drawIf('e', 1 << 2);
  drawIf('f', 1 << 1);
  drawIf('g', 1 << 0);
}

void Renderer::drawNumber(SDL_Renderer* r, const SDL_Rect& rect,
                          int value) const {
  if (value <= 0) return;

  std::ostringstream ss;
  ss << value;
  const std::string s = ss.str();

  const SDL_Color color = textColorFor(value);

  // Fit digits inside rect.
  const int maxDigits = static_cast<int>(s.size());
  const int digitW = std::max(10, rect.w / std::max(2, maxDigits));
  const int digitH = std::max(18, rect.h / 2);
  const int totalW = digitW * maxDigits + (maxDigits - 1) * (digitW / 6);
  int x = rect.x + (rect.w - totalW) / 2;
  const int y = rect.y + (rect.h - digitH) / 2;

  for (char ch : s) {
    const int d = ch - '0';
    drawDigit(r, d, x, y, digitW, digitH, color);
    x += digitW + (digitW / 6);
  }
}

void Renderer::render(SDL_Renderer* r, const Game& game,
                      const std::unordered_map<int, Tile>& tiles, int windowW,
                      int windowH, bool gameOver, int score) {
  // Background
  setColor(r, Utils::backgroundPink());
  SDL_RenderClear(r);

  // Board base
  const SDL_Rect b = boardRect(windowW, windowH);
  fillRoundRect(r, b, 16, SDL_Color{255, 255, 255, 35});

  // Empty cells
  for (int rr = 0; rr < 4; ++rr) {
    for (int cc = 0; cc < 4; ++cc) {
      SDL_Rect cell = cellRect(windowW, windowH, static_cast<float>(rr),
                               static_cast<float>(cc));
      fillRoundRect(r, cell, 12, Utils::gridEmptyCellColor());
    }
  }

  // Tiles
  // Draw in value order so larger tiles appear on top (helps pop look).
  std::vector<const Tile*> drawList;
  drawList.reserve(tiles.size());
  for (const auto& kv : tiles) drawList.push_back(&kv.second);
  std::sort(drawList.begin(), drawList.end(),
            [](const Tile* a, const Tile* b) { return a->value() < b->value(); });

  for (const Tile* t : drawList) {
    if (t->value() <= 0) continue;

    float row = 0.0f, col = 0.0f;
    t->interpolatedPos(row, col);
    SDL_Rect base = cellRect(windowW, windowH, row, col);

    const float scale = t->popScale();
    SDL_Rect rect = base;
    if (scale != 1.0f) {
      const int cx = base.x + base.w / 2;
      const int cy = base.y + base.h / 2;
      rect.w = static_cast<int>(std::round(base.w * scale));
      rect.h = static_cast<int>(std::round(base.h * scale));
      rect.x = cx - rect.w / 2;
      rect.y = cy - rect.h / 2;
    }

    fillRoundRect(r, rect, 12, Utils::tileColor(t->value()));

    // Border
    setColor(r, Utils::tileBorderColor());
    SDL_RenderDrawRect(r, &rect);

    drawNumber(r, rect, t->value());
  }

  if (gameOver) {
    // Simple overlay
    setColor(r, SDL_Color{0, 0, 0, 120});
    SDL_RenderFillRect(r, &b);
    // "GAME OVER" using digits isn't possible; show 2048-style by drawing 0
    // and rely on instructions (R to restart). Keep minimal.
  }

  // Draw score in top right
  {
    const int scoreX = windowW - 200;
    const int scoreY = 20;
    const int scoreW = 180;
    const int scoreH = 40;
    SDL_Rect scoreRect{scoreX, scoreY, scoreW, scoreH};
    drawNumber(r, scoreRect, score);
  }

  SDL_RenderPresent(r);
}


