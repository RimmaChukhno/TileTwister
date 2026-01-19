#include "Game.hpp"

#include "Utils.hpp"

#include <array>
#include <cstring>

namespace {

struct LineItem {
  int value = 0;
  int srcIndex = -1; 
};

struct LineMoveOut {
  std::array<int, 4> values{0, 0, 0, 0};
  std::vector<std::pair<int, int>> srcToDst; 
  std::vector<int> mergedDst;                
  bool changed = false;
};

// Processes a line of 4 cells in "forward" direction (index 0 is the side
// tiles are moving towards). Produces new values and a movement mapping.
LineMoveOut moveLineForward(const std::array<int, 4>& in) {
  LineMoveOut out;
  std::vector<LineItem> items;
  items.reserve(4);
  for (int i = 0; i < 4; ++i) {
    if (in[i] != 0) items.push_back(LineItem{in[i], i});
  }

  int write = 0;
  int i = 0;
  while (i < static_cast<int>(items.size())) {
    if (i + 1 < static_cast<int>(items.size()) &&
        items[i].value == items[i + 1].value) {
      // Merge: two tiles go into one cell.
      out.values[write] = items[i].value * 2;
      out.srcToDst.emplace_back(items[i].srcIndex, write);
      out.srcToDst.emplace_back(items[i + 1].srcIndex, write);
      out.mergedDst.push_back(write);
      ++write;
      i += 2;
    } else {
      out.values[write] = items[i].value;
      out.srcToDst.emplace_back(items[i].srcIndex, write);
      ++write;
      i += 1;
    }
  }

  // Detect if anything changed in this line.
  for (int j = 0; j < 4; ++j) {
    if (out.values[j] != in[j]) {
      out.changed = true;
      break;
    }
  }
  return out;
}

} // namespace

Game::Game() { reset(); }

void Game::clearGrid() { std::memset(m_grid, 0, sizeof(m_grid)); }

void Game::spawnInitial() {
  // Spawn two tiles on a fresh board.
  for (int i = 0; i < 2; ++i) {
    auto sp = rollSpawn(m_grid);
    if (!sp) break;
    m_grid[sp->first.r][sp->first.c] = sp->second;
  }
}

void Game::reset() {
  clearGrid();
  m_pendingSpawn.reset();
  m_score.reset();
  spawnInitial();
}

std::optional<std::pair<Cell, int>> Game::rollSpawn(const int grid[4][4]) const {
  const auto empties = Utils::emptyCells(grid);
  if (empties.empty()) return std::nullopt;
  const int idx = Utils::randInt(0, static_cast<int>(empties.size()) - 1);
  const int value = Utils::chance(1, 10) ? 4 : 2; // 10% 4, 90% 2
  return std::make_pair(Cell{empties[idx].first, empties[idx].second}, value);
}

MoveResult Game::tryMove(Direction dir) {
  MoveResult res;
  res.moved = false;
  res.animations.clear();
  res.mergedCells.clear();
  res.pendingSpawn.reset();

  // If we still have an uncommitted spawn, don't allow moving again.
  if (m_pendingSpawn.has_value()) return res;

  int outGrid[4][4]{};
  std::memset(outGrid, 0, sizeof(outGrid));

  auto addAnim = [&](int srcLine, int srcIdx, int dstLine, int dstIdx,
                     bool isRowLine) {
    Cell from, to;
    if (isRowLine) {
      from = Cell{srcLine, srcIdx};
      to = Cell{dstLine, dstIdx};
    } else {
      from = Cell{srcIdx, srcLine};
      to = Cell{dstIdx, dstLine};
    }
    const int v = m_grid[from.r][from.c];
    if (v != 0) res.animations.push_back(MoveAnim{from, to, v});
  };

  const bool isRowLine = (dir == Direction::Left || dir == Direction::Right);

  for (int line = 0; line < 4; ++line) {
    std::array<int, 4> in{};
    for (int i = 0; i < 4; ++i) {
      if (isRowLine) {
        const int c = (dir == Direction::Left) ? i : (3 - i);
        in[i] = m_grid[line][c];
      } else {
        const int r = (dir == Direction::Up) ? i : (3 - i);
        in[i] = m_grid[r][line];
      }
    }

    const LineMoveOut out = moveLineForward(in);
    if (out.changed) res.moved = true;

    // Write back to outGrid and build animations.
    for (int i = 0; i < 4; ++i) {
      if (isRowLine) {
        const int c = (dir == Direction::Left) ? i : (3 - i);
        outGrid[line][c] = out.values[i];
      } else {
        const int r = (dir == Direction::Up) ? i : (3 - i);
        outGrid[r][line] = out.values[i];
      }
    }

    // Animations: map line indices back to board coords.
    for (const auto& p : out.srcToDst) {
      const int srcIdxFwd = p.first;
      const int dstIdxFwd = p.second;
      // Convert forward indices back to absolute indices in the board line.
      const int srcAbs =
          (dir == Direction::Left || dir == Direction::Up) ? srcIdxFwd
                                                          : (3 - srcIdxFwd);
      const int dstAbs =
          (dir == Direction::Left || dir == Direction::Up) ? dstIdxFwd
                                                          : (3 - dstIdxFwd);
      addAnim(line, srcAbs, line, dstAbs, isRowLine);
    }

    for (int mergedDstFwd : out.mergedDst) {
      const int dstAbs =
          (dir == Direction::Left || dir == Direction::Up) ? mergedDstFwd
                                                          : (3 - mergedDstFwd);
      if (isRowLine) {
        res.mergedCells.push_back(Cell{line, dstAbs});
      } else {
        res.mergedCells.push_back(Cell{dstAbs, line});
      }
      m_score.addScore(out.values[mergedDstFwd]);
    }
  }

  if (!res.moved) return res;

  // Apply post-move grid (pre-spawn) immediately.
  std::memcpy(m_grid, outGrid, sizeof(m_grid));

  // Roll a spawn but don't apply yet (so visuals can spawn after slide).
  m_pendingSpawn = rollSpawn(m_grid);
  res.pendingSpawn = m_pendingSpawn;

  return res;
}

bool Game::commitPendingSpawn() {
  if (!m_pendingSpawn.has_value()) return false;
  const Cell cell = m_pendingSpawn->first;
  const int value = m_pendingSpawn->second;
  if (m_grid[cell.r][cell.c] == 0) {
    m_grid[cell.r][cell.c] = value;
  }
  m_pendingSpawn.reset();
  return true;
}

bool Game::hasAnyMove(const int grid[4][4]) const {
  // Any empty?
  for (int r = 0; r < 4; ++r)
    for (int c = 0; c < 4; ++c)
      if (grid[r][c] == 0) return true;

  // Any merge neighbor?
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      const int v = grid[r][c];
      if (r + 1 < 4 && grid[r + 1][c] == v) return true;
      if (c + 1 < 4 && grid[r][c + 1] == v) return true;
    }
  }
  return false;
}

bool Game::isGameOver() const { return !hasAnyMove(m_grid); }

void Game::setGridForTest(const int grid[4][4]) {
  std::memcpy(m_grid, grid, sizeof(m_grid));
  m_pendingSpawn.reset();
}

void Game::clearPendingSpawnForTest() { m_pendingSpawn.reset(); }


