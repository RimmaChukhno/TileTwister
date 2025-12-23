#include <tiletwister/app/GameControllerObject.hpp>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstring>

GameControllerObject::GameControllerObject(bool* runningFlag)
    : m_running(runningFlag) {
  rebuildTilesFromGrid();
}

int GameControllerObject::keyForCellValue(int r, int c, int value, int ordinal) {
  return (r & 0xF) | ((c & 0xF) << 4) | ((value & 0xFFFF) << 8) |
         ((ordinal & 0xFF) << 24);
}

void GameControllerObject::rebuildTilesFromGrid() {
  m_tiles.clear();
  int counts[16]{};
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      const int v = m_game.grid()[r][c];
      if (v == 0) continue;
      const int idx = r * 4 + c;
      const int ord = counts[idx]++;
      const int k = keyForCellValue(r, c, v, ord);
      m_tiles.emplace(k, Tile(v, Cell{r, c}));
    }
  }
}

void GameControllerObject::beginMove(Direction dir) {
  if (m_activeMove.active) return;

  int before[4][4]{};
  std::memcpy(before, m_game.grid(), sizeof(before));

  const MoveResult mr = m_game.tryMove(dir);
  if (!mr.moved) return;

  if (m_game.score() > m_bestScore) m_bestScore = m_game.score();

  m_tiles.clear();

  // Build tiles from animation sources.
  int srcSeen[4][4]{};
  for (const auto& a : mr.animations) {
    const int ord = srcSeen[a.from.r][a.from.c]++;
    const int k = keyForCellValue(a.from.r, a.from.c, a.value, ord);
    Tile t(a.value, a.from);
    t.startSlide(a.from, a.to, m_activeMove.duration);
    m_tiles.emplace(k, t);
  }

  // Add stationary tiles (ones not referenced as a move source).
  bool usedFrom[4][4]{};
  for (const auto& a : mr.animations) usedFrom[a.from.r][a.from.c] = true;
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      const int v = before[r][c];
      if (v == 0) continue;
      if (usedFrom[r][c]) continue;
      const int k = keyForCellValue(r, c, v, 0);
      m_tiles.emplace(k, Tile(v, Cell{r, c}));
    }
  }

  m_activeMove.active = true;
  m_activeMove.timeLeft = m_activeMove.duration;

  m_activeMove.pendingPopCells = mr.mergedCells;
  if (mr.pendingSpawn.has_value())
    m_activeMove.pendingSpawnCell = mr.pendingSpawn->first;
  else
    m_activeMove.pendingSpawnCell.reset();
}

void GameControllerObject::handleEvent(const SDL_Event& e) {
  if (e.type != SDL_KEYDOWN) return;

  const SDL_Keycode key = e.key.keysym.sym;
  if (key == SDLK_ESCAPE) {
    if (m_running) *m_running = false;
    return;
  }

  if (key == SDLK_r) {
    m_game.reset();
    // Keep best score across resets.
    m_activeMove.active = false;
    m_game.commitPendingSpawn();
    rebuildTilesFromGrid();
    return;
  }

  if (m_activeMove.active) return;

  if (key == SDLK_LEFT) beginMove(Direction::Left);
  else if (key == SDLK_RIGHT) beginMove(Direction::Right);
  else if (key == SDLK_UP) beginMove(Direction::Up);
  else if (key == SDLK_DOWN) beginMove(Direction::Down);
}

void GameControllerObject::update(float dtSec) {
  for (auto& kv : m_tiles) kv.second.update(dtSec);

  if (m_game.score() > m_bestScore) m_bestScore = m_game.score();

  if (!m_activeMove.active) return;

  m_activeMove.timeLeft -= dtSec;
  if (m_activeMove.timeLeft > 0.0f) return;

  m_activeMove.active = false;

  // Commit spawn and rebuild final board tiles.
  m_game.commitPendingSpawn();
  rebuildTilesFromGrid();

  // Pop merged destinations.
  for (const Cell& c : m_activeMove.pendingPopCells) {
    for (auto& kv : m_tiles) {
      if (kv.second.cell().r == c.r && kv.second.cell().c == c.c) {
        kv.second.startPop(0.10f);
      }
    }
  }

  // Pop newly spawned tile (if any).
  if (m_activeMove.pendingSpawnCell.has_value()) {
    const Cell c = *m_activeMove.pendingSpawnCell;
    for (auto& kv : m_tiles) {
      if (kv.second.cell().r == c.r && kv.second.cell().c == c.c) {
        kv.second.startPop(0.12f);
      }
    }
  }

  m_activeMove.pendingPopCells.clear();
  m_activeMove.pendingSpawnCell.reset();
}

void GameControllerObject::render(SDL_Renderer* renderer) {
  m_renderer.render(renderer, m_game, m_tiles, m_windowW, m_windowH,
                    m_game.score(), m_bestScore, m_game.isGameOver());
}


