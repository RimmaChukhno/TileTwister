#pragma once

#include <tiletwister/game/Game.hpp>

struct TileAnim {
  bool active = false;
  float t = 1.0f;         // 0..1
  float duration = 0.12f; // seconds
  Cell from{0, 0};
  Cell to{0, 0};
};

struct PopAnim {
  bool active = false;
  float t = 1.0f;         // 0..1
  float duration = 0.10f; // seconds
};

class Tile {
public:
  Tile() = default;
  Tile(int value, Cell cell);

  void setValue(int v) { m_value = v; }
  int value() const { return m_value; }

  void setCell(Cell c) { m_cell = c; }
  Cell cell() const { return m_cell; }

  // Animation controls
  void startSlide(Cell from, Cell to, float durationSec);
  void startPop(float durationSec);
  void update(float dtSec);

  // Visual helpers
  bool isSliding() const { return m_slide.active; }
  bool isPopping() const { return m_pop.active; }

  // Returns interpolated grid position in continuous coordinates (row/col floats)
  void interpolatedPos(float& outRow, float& outCol) const;

  // Returns scale multiplier for pop effect (1.0 = normal)
  float popScale() const;

private:
  int m_value = 0;
  Cell m_cell{0, 0}; // logical cell (where tile "ends up" in the grid)

  TileAnim m_slide{};
  PopAnim m_pop{};
};


