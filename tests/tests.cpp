#include <tiletwister/game/Game.hpp>
#include <tiletwister/core/Utils.hpp>

#include <cassert>
#include <cstring>
#include <iostream>

static void expectGridEq(const Game& g, const int expected[4][4]) {
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (g.grid()[r][c] != expected[r][c]) {
        std::cerr << "Mismatch at (" << r << "," << c << "): got "
                  << g.grid()[r][c] << " expected " << expected[r][c] << "\n";
        assert(false);
      }
    }
  }
}

static void testMoveLeftSimpleMerge() {
  Game g;
  g.clearPendingSpawnForTest();
  const int in[4][4] = {
      {2, 0, 2, 2},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
  g.setGridForTest(in);
  const MoveResult mr = g.tryMove(Direction::Left);
  assert(mr.moved);
  const int expected[4][4] = {
      {4, 2, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
  expectGridEq(g, expected);
}

static void testMoveLeftMergeOnceRule() {
  Game g;
  const int in[4][4] = {
      {2, 2, 2, 2},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
  g.setGridForTest(in);
  const MoveResult mr = g.tryMove(Direction::Left);
  assert(mr.moved);
  const int expected[4][4] = {
      {4, 4, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
  expectGridEq(g, expected);
}

static void testMoveRightDoubleMerge() {
  Game g;
  const int in[4][4] = {
      {2, 2, 4, 4},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
  g.setGridForTest(in);
  const MoveResult mr = g.tryMove(Direction::Right);
  assert(mr.moved);
  const int expected[4][4] = {
      {0, 0, 4, 8},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
  expectGridEq(g, expected);
}

static void testMoveUpColumnMerge() {
  Game g;
  const int in[4][4] = {
      {2, 0, 0, 0},
      {0, 0, 0, 0},
      {2, 0, 0, 0},
      {2, 0, 0, 0},
  };
  g.setGridForTest(in);
  const MoveResult mr = g.tryMove(Direction::Up);
  assert(mr.moved);
  const int expected[4][4] = {
      {4, 0, 0, 0},
      {2, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
  expectGridEq(g, expected);
}

static void testGameOverDetection() {
  Game g;
  const int in[4][4] = {
      {2, 4, 2, 4},
      {4, 2, 4, 2},
      {2, 4, 2, 4},
      {4, 2, 4, 2},
  };
  g.setGridForTest(in);
  assert(g.isGameOver());
}

static void testSpawnPendingAndCommit() {
  Utils::rng().seed(12345); // deterministic
  Game g;
  const int in[4][4] = {
      {2, 2, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
  };
  g.setGridForTest(in);
  const MoveResult mr = g.tryMove(Direction::Left);
  assert(mr.moved);
  assert(mr.pendingSpawn.has_value());
  const int v = mr.pendingSpawn->second;
  assert(v == 2 || v == 4);

  const Cell c = mr.pendingSpawn->first;
  assert(g.grid()[c.r][c.c] == 0); // pre-commit

  const bool spawned = g.commitPendingSpawn();
  assert(spawned);
  assert(g.grid()[c.r][c.c] == v);
}

int main() {
  testMoveLeftSimpleMerge();
  testMoveLeftMergeOnceRule();
  testMoveRightDoubleMerge();
  testMoveUpColumnMerge();
  testGameOverDetection();
  testSpawnPendingAndCommit();
  std::cout << "All tests passed.\n";
  return 0;
}


