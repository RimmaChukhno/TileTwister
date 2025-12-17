#pragma once

#include <optional>
#include <utility>
#include <vector>

enum class Direction { Left, Right, Up, Down };

struct Cell {
  int r = 0;
  int c = 0;
};

struct MoveAnim {
  // Source cell -> destination cell for one visual tile.
  Cell from;
  Cell to;
  int value = 0;
};

struct MoveResult {
  bool moved = false;
  std::vector<MoveAnim> animations;     // tiles that visually slide
  std::vector<Cell> mergedCells;        // destination cells that merged (pop)
  std::optional<std::pair<Cell, int>> pendingSpawn; // apply after slide ends
};

class Game {
public:
  Game();

  void reset();

  // Returns the move result; when moved==true the internal grid is updated
  // to the post-merge state (but before spawning the new tile).
  MoveResult tryMove(Direction dir);

  // Call after finishing move animation to actually spawn the pending tile.
  // Returns true if a tile was spawned.
  bool commitPendingSpawn();

  bool isGameOver() const;

  const int (&grid() const)[4][4] { return m_grid; }

  // Test helpers (logic-only; not used by the SDL gameplay loop).
  void setGridForTest(const int grid[4][4]);
  void clearPendingSpawnForTest();

private:
  int m_grid[4][4]{};
  std::optional<std::pair<Cell, int>> m_pendingSpawn;

  void clearGrid();
  void spawnInitial();
  std::optional<std::pair<Cell, int>> rollSpawn(const int grid[4][4]) const;

  bool hasAnyMove(const int grid[4][4]) const;
};


