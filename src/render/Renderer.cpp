#include <tiletwister/render/Renderer.hpp>

#include <tiletwister/game/Game.hpp>
#include <tiletwister/game/Tile.hpp>
#include <tiletwister/render/Palette.hpp>

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

// Minimal 5x7 bitmap font for the HUD labels.
// Only includes characters needed for "SCORE" and "BEST".
const char* glyph5x7(char ch) {
  switch (ch) {
  case '0':
    return "01110"
           "10001"
           "10011"
           "10101"
           "11001"
           "10001"
           "01110";
  case '1':
    return "00100"
           "01100"
           "00100"
           "00100"
           "00100"
           "00100"
           "01110";
  case '2':
    return "01110"
           "10001"
           "00001"
           "00010"
           "00100"
           "01000"
           "11111";
  case '3':
    return "11110"
           "00001"
           "00001"
           "01110"
           "00001"
           "00001"
           "11110";
  case '4':
    return "00010"
           "00110"
           "01010"
           "10010"
           "11111"
           "00010"
           "00010";
  case '5':
    return "11111"
           "10000"
           "10000"
           "11110"
           "00001"
           "00001"
           "11110";
  case '6':
    return "01110"
           "10000"
           "10000"
           "11110"
           "10001"
           "10001"
           "01110";
  case '7':
    return "11111"
           "00001"
           "00010"
           "00100"
           "01000"
           "01000"
           "01000";
  case '8':
    return "01110"
           "10001"
           "10001"
           "01110"
           "10001"
           "10001"
           "01110";
  case '9':
    return "01110"
           "10001"
           "10001"
           "01111"
           "00001"
           "00001"
           "01110";
  case 'S':
    return "01111"
           "10000"
           "10000"
           "01110"
           "00001"
           "00001"
           "11110";
  case 'C':
    return "01111"
           "10000"
           "10000"
           "10000"
           "10000"
           "10000"
           "01111";
  case 'O':
    return "01110"
           "10001"
           "10001"
           "10001"
           "10001"
           "10001"
           "01110";
  case 'R':
    return "11110"
           "10001"
           "10001"
           "11110"
           "10100"
           "10010"
           "10001";
  case 'E':
    return "11111"
           "10000"
           "10000"
           "11110"
           "10000"
           "10000"
           "11111";
  case 'B':
    return "11110"
           "10001"
           "10001"
           "11110"
           "10001"
           "10001"
           "11110";
  case 'T':
    return "11111"
           "00100"
           "00100"
           "00100"
           "00100"
           "00100"
           "00100";
  case 'L':
    return "10000"
           "10000"
           "10000"
           "10000"
           "10000"
           "10000"
           "11111";
  case 'J':
    return "00111"
           "00010"
           "00010"
           "00010"
           "00010"
           "10010"
           "01100";
  case 'U':
    return "10001"
           "10001"
           "10001"
           "10001"
           "10001"
           "10001"
           "01110";
  case 'M':
    return "10001"
           "11011"
           "10101"
           "10101"
           "10001"
           "10001"
           "10001";
  case 'N':
    return "10001"
           "11001"
           "10101"
           "10011"
           "10001"
           "10001"
           "10001";
  case 'I':
    return "01110"
           "00100"
           "00100"
           "00100"
           "00100"
           "00100"
           "01110";
  case '?':
    return "01110"
           "10001"
           "00001"
           "00010"
           "00100"
           "00000"
           "00100";
  case ' ':
  default:
    return "00000"
           "00000"
           "00000"
           "00000"
           "00000"
           "00000"
           "00000";
  }
}

void drawText5x7(SDL_Renderer* r, const std::string& text, const SDL_Rect& rect,
                 SDL_Color color) {
  setColor(r, color);
  const int cols = 5;
  const int rows = 7;
  const int gapPx = std::max(1, rect.w / 80);
  const int charGap = std::max(1, gapPx * 2);

  const int n = static_cast<int>(text.size());
  if (n <= 0) return;

  // Compute pixel size so the text fits.
  const int availW = std::max(1, rect.w);
  const int availH = std::max(1, rect.h);
  const int cellW = std::max(1, (availW - (n - 1) * charGap) / (n * cols));
  const int cellH = std::max(1, availH / rows);
  const int cell = std::min(cellW, cellH);

  const int textW = n * cols * cell + (n - 1) * charGap;
  const int textH = rows * cell;
  int x0 = rect.x + (rect.w - textW) / 2;
  int y0 = rect.y + (rect.h - textH) / 2;

  for (int i = 0; i < n; ++i) {
    const char up =
        (text[i] >= 'a' && text[i] <= 'z') ? (text[i] - 'a' + 'A') : text[i];
    const char* g = glyph5x7(up);
    for (int ry = 0; ry < rows; ++ry) {
      for (int cx = 0; cx < cols; ++cx) {
        const char bit = g[ry * cols + cx];
        if (bit == '1') {
          SDL_Rect px{ x0 + i * (cols * cell + charGap) + cx * cell,
                       y0 + ry * cell,
                       cell, cell };
          SDL_RenderFillRect(r, &px);
        }
      }
    }
  }
}

} // namespace

SDL_Rect Renderer::computeBoardRect(int windowW, int windowH) {
  // Reserve space at the top for the HUD (score/best).
  const int outerMargin = 40;
  const int hudH = 80;
  const int availW = windowW - 2 * outerMargin;
  const int availH = windowH - hudH - 2 * outerMargin;
  const int size = std::max(200, std::min(availW, availH));
  const int x = (windowW - size) / 2;
  const int y = hudH + outerMargin;
  return SDL_Rect{x, y, size, size};
}

SDL_Rect Renderer::computeGameOverPanelRect(int windowW, int windowH) {
  const SDL_Rect b = computeBoardRect(windowW, windowH);
  const int w = static_cast<int>(std::round(b.w * 0.86f));
  const int h = static_cast<int>(std::round(b.h * 0.42f));
  const int x = b.x + (b.w - w) / 2;
  const int y = b.y + (b.h - h) / 2;
  return SDL_Rect{x, y, w, h};
}

SDL_Rect Renderer::computeGameOverButtonRect(int windowW, int windowH) {
  const SDL_Rect p = computeGameOverPanelRect(windowW, windowH);
  const int pad = std::max(12, p.w / 20);
  const int btnH = std::max(40, p.h / 4);
  const int x = p.x + pad;
  const int w = p.w - 2 * pad;
  const int y = p.y + p.h - pad - btnH;
  return SDL_Rect{x, y, w, btnH};
}

SDL_Rect Renderer::boardRect(int windowW, int windowH) const {
  return computeBoardRect(windowW, windowH);
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
  // Simple approximation: fill rect.
  // (Keeping dependencies minimal.)
  setColor(r, color);
  SDL_RenderFillRect(r, &rect);
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

  // Segment thickness scales with both width and height so it doesn't bloat
  // when digits get narrow (e.g. 4+ digits in a tile).
  const int minSide = std::max(1, std::min(w, h));
  const int t = std::max(1, minSide / 7); // segment thickness
  const int pad = std::max(1, t);         // inner padding

  auto segRect = [&](char seg) -> SDL_Rect {
    switch (seg) {
    case 'a':
      return SDL_Rect{x + pad, y + pad, w - 2 * pad, t};
    case 'd':
      return SDL_Rect{x + pad, y + h - t - pad, w - 2 * pad, t};
    case 'g':
      return SDL_Rect{x + pad, y + (h - t) / 2, w - 2 * pad, t};
    case 'f':
      return SDL_Rect{x + pad, y + pad, t, (h - 3 * pad) / 2};
    case 'b':
      return SDL_Rect{x + w - t - pad, y + pad, t, (h - 3 * pad) / 2};
    case 'e':
      return SDL_Rect{x + pad, y + (h + pad) / 2, t, (h - 3 * pad) / 2};
    case 'c':
      return SDL_Rect{x + w - t - pad, y + (h + pad) / 2, t,
                      (h - 3 * pad) / 2};
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

  // Fit digits inside rect with explicit padding so numbers never overflow.
  const int digits = static_cast<int>(s.size());
  const int pad = std::max(6, rect.w / 10);
  const int availW = std::max(1, rect.w - 2 * pad);
  const int availH = std::max(1, rect.h - 2 * pad);

  // Gap between digits scales down for narrow layouts.
  const int gap = std::max(2, availW / 40);

  // Compute digit width so the whole string fits horizontally.
  const int digitW = std::max(6, (availW - gap * (digits - 1)) / digits);

  // Height: keep a 7-seg aspect (roughly 2:1), but clamp to available height.
  const int digitH = std::max(10, std::min(availH, digitW * 2));

  const int totalW = digitW * digits + gap * (digits - 1);
  int x = rect.x + (rect.w - totalW) / 2;
  const int y = rect.y + (rect.h - digitH) / 2;

  for (char ch : s) {
    const int d = ch - '0';
    drawDigit(r, d, x, y, digitW, digitH, color);
    x += digitW + gap;
  }
}

void Renderer::render(SDL_Renderer* r, const Game& game,
                      const std::unordered_map<int, Tile>& tiles, int windowW,
                      int windowH, int score, int bestScore, bool gameOver,
                      bool gameOverButtonHover) {
  (void)game;
  // Background
  setColor(r, Palette::backgroundPink());
  SDL_RenderClear(r);

  // Board base
  const SDL_Rect b = boardRect(windowW, windowH);

  // HUD (top area)
  {
    const int hudTop = 12;
    const int hudBottom = std::max(hudTop + 10, b.y - 12);
    const int hudH = std::max(40, hudBottom - hudTop);
    const SDL_Rect hudArea{b.x, hudTop, b.w, hudH};

    const int gap = 12;
    const int boxW = (hudArea.w - gap) / 2;
    const SDL_Rect scoreBox{hudArea.x, hudArea.y, boxW, hudArea.h};
    const SDL_Rect bestBox{hudArea.x + boxW + gap, hudArea.y, boxW, hudArea.h};

    fillRoundRect(r, scoreBox, 12, SDL_Color{255, 255, 255, 45});
    fillRoundRect(r, bestBox, 12, SDL_Color{255, 255, 255, 45});

    const int pad = 10;
    const int labelH = std::max(16, scoreBox.h / 3);
    // Give extra room so score digits don't look crushed.
    const int numPad = pad + 4;
    const int numTopGap = 6;

    SDL_Rect scoreLabel{scoreBox.x + pad, scoreBox.y + pad,
                        scoreBox.w - 2 * pad, labelH};
    SDL_Rect bestLabel{bestBox.x + pad, bestBox.y + pad,
                       bestBox.w - 2 * pad, labelH};
    SDL_Rect scoreNum{
        scoreBox.x + numPad,
        scoreBox.y + pad + labelH + numTopGap,
        scoreBox.w - 2 * numPad,
        scoreBox.h - (pad + labelH + numTopGap) - numPad,
    };
    SDL_Rect bestNum{
        bestBox.x + numPad,
        bestBox.y + pad + labelH + numTopGap,
        bestBox.w - 2 * numPad,
        bestBox.h - (pad + labelH + numTopGap) - numPad,
    };

    const SDL_Color labelColor{80, 40, 60, 255};
    drawText5x7(r, "SCORE", scoreLabel, labelColor);
    drawText5x7(r, "BEST", bestLabel, labelColor);

    // HUD numbers: square pixel font so they stay readable and compact.
    drawText5x7(r, std::to_string(score), scoreNum, labelColor);
    drawText5x7(r, std::to_string(bestScore), bestNum, labelColor);
  }

  fillRoundRect(r, b, 16, SDL_Color{255, 255, 255, 35});

  // Empty cells
  for (int rr = 0; rr < 4; ++rr) {
    for (int cc = 0; cc < 4; ++cc) {
      SDL_Rect cell = cellRect(windowW, windowH, static_cast<float>(rr),
                               static_cast<float>(cc));
      fillRoundRect(r, cell, 12, Palette::gridEmptyCellColor());
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

    fillRoundRect(r, rect, 12, Palette::tileColor(t->value()));

    // Border
    setColor(r, Palette::tileBorderColor());
    SDL_RenderDrawRect(r, &rect);

    drawNumber(r, rect, t->value());
  }

  if (gameOver) {
    // Dim the board and show a centered "window" with restart button.
    setColor(r, SDL_Color{0, 0, 0, 120});
    SDL_RenderFillRect(r, &b);

    const SDL_Rect panel = computeGameOverPanelRect(windowW, windowH);
    fillRoundRect(r, panel, 14, SDL_Color{255, 255, 255, 70});

    // Message (no accents in bitmap font): "LE JEU EST TERMINE"
    const int pad = std::max(12, panel.w / 20);
    SDL_Rect line1{panel.x + pad, panel.y + pad, panel.w - 2 * pad,
                   std::max(18, panel.h / 5)};
    SDL_Rect line2{panel.x + pad, line1.y + line1.h + 6, panel.w - 2 * pad,
                   std::max(18, panel.h / 5)};

    const SDL_Color msgColor{80, 40, 60, 255};
    drawText5x7(r, "LE JEU EST", line1, msgColor);
    drawText5x7(r, "TERMINE", line2, msgColor);

    // Button
    const SDL_Rect btn = computeGameOverButtonRect(windowW, windowH);
    const SDL_Color btnFill = gameOverButtonHover ? SDL_Color{255, 255, 255, 110}
                                                  : SDL_Color{255, 255, 255, 80};
    fillRoundRect(r, btn, 12, btnFill);
    setColor(r, SDL_Color{80, 40, 60, 80});
    SDL_RenderDrawRect(r, &btn);

    SDL_Rect btnText{btn.x + 10, btn.y + 6, btn.w - 20, btn.h - 12};
    drawText5x7(r, "RECOMMENCER ?", btnText, msgColor);
  }

  SDL_RenderPresent(r);
}


