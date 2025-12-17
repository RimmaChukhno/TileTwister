#include <tiletwister/game/Tile.h>

#include <tiletwister/core/Utils.h>

Tile::Tile(int value, Cell cell) : m_value(value), m_cell(cell) {}

void Tile::startSlide(Cell from, Cell to, float durationSec) {
  m_slide.active = true;
  m_slide.t = 0.0f;
  m_slide.duration = (durationSec <= 0.0f) ? 0.001f : durationSec;
  m_slide.from = from;
  m_slide.to = to;
  // Logical cell is where it'll land.
  m_cell = to;
}

void Tile::startPop(float durationSec) {
  m_pop.active = true;
  m_pop.t = 0.0f;
  m_pop.duration = (durationSec <= 0.0f) ? 0.001f : durationSec;
}

void Tile::update(float dtSec) {
  if (m_slide.active) {
    m_slide.t += dtSec / m_slide.duration;
    if (m_slide.t >= 1.0f) {
      m_slide.t = 1.0f;
      m_slide.active = false;
    }
  }
  if (m_pop.active) {
    m_pop.t += dtSec / m_pop.duration;
    if (m_pop.t >= 1.0f) {
      m_pop.t = 1.0f;
      m_pop.active = false;
    }
  }
}

void Tile::interpolatedPos(float& outRow, float& outCol) const {
  if (!m_slide.active) {
    outRow = static_cast<float>(m_cell.r);
    outCol = static_cast<float>(m_cell.c);
    return;
  }
  const float t = Utils::easeOutCubic(m_slide.t);
  outRow = Utils::lerp(static_cast<float>(m_slide.from.r),
                       static_cast<float>(m_slide.to.r), t);
  outCol = Utils::lerp(static_cast<float>(m_slide.from.c),
                       static_cast<float>(m_slide.to.c), t);
}

float Tile::popScale() const {
  if (!m_pop.active) return 1.0f;
  // Quick overshoot: 1 -> 1.12 -> 1 (ease out/in feel)
  const float t = Utils::clampf(m_pop.t, 0.0f, 1.0f);
  const float up = (t < 0.5f) ? (t / 0.5f) : ((1.0f - t) / 0.5f);
  return 1.0f + 0.12f * Utils::easeOutCubic(up);
}


