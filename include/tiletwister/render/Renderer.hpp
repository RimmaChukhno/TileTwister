#pragma once

#include <SDL2/SDL.h>

#include <string>
#include <unordered_map>

class Game;
class Tile;

// Lightweight renderer:
// - draws background, board, empty cells, tiles
// - draws numbers using a tiny built-in 7-seg style (no SDL_ttf dependency)
class Renderer {
public:
  Renderer() = default;
  ~Renderer() = default;

  void render(SDL_Renderer* r, const Game& game,
              const std::unordered_map<int, Tile>& tiles, int windowW,
              int windowH, int score, int bestScore, bool gameOver);

private:
  // Layout helpers
  SDL_Rect boardRect(int windowW, int windowH) const;
  SDL_Rect cellRect(int windowW, int windowH, float row, float col) const;

  // Primitives
  void fillRoundRect(SDL_Renderer* r, const SDL_Rect& rect, int radius,
                     SDL_Color color) const;

  // Number drawing (7 segment)
  void drawNumber(SDL_Renderer* r, const SDL_Rect& rect, int value) const;
  void drawDigit(SDL_Renderer* r, int digit, int x, int y, int w, int h,
                 SDL_Color color) const;
};


