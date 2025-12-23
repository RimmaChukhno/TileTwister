#include <tiletwister/game/Game.hpp>
#include <tiletwister/core/Utils.hpp>
#include <tiletwister/engine/Scene.hpp>

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

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

// Verifies a LEFT move:
// - compacts tiles toward the left
// - merges equal values once (2 + 2 => 4)
// - preserves remaining tile order (final 2 stays after the merged 4)
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

// Verifies the classic 2048 rule "a tile can merge only once per move":
// [2,2,2,2] moving LEFT becomes [4,4,0,0] (not [8,0,0,0]).
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

// Verifies RIGHT move with two independent merges on the same row:
// [2,2,4,4] -> [0,0,4,8].
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

// Verifies UP move + merge behavior in a column:
// tiles must compact upward, merge once, and keep leftover below.
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

// Verifies game-over detection on a full grid with no possible merges.
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

// Verifies the spawn pipeline:
// - after a valid move, Game produces a pending spawn (2 or 4) in an empty cell
// - commitPendingSpawn() applies exactly that spawn cell/value.
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

// Integration test for the "engine layer":
// validates that Scene:
// - calls update() in insertion order
// - removes objects that report alive()==false after update
// - renders objects in zIndex() order
static void testIntegrationSceneLifecycleAndOrdering() {
  struct TraceObject final : public GameObject {
    std::string name;
    int z = 0;
    bool isAlive = true;
    std::vector<std::string>* log = nullptr;

    TraceObject(std::string n, int zIndex, std::vector<std::string>* l,
                bool alive = true)
        : name(std::move(n)), z(zIndex), isAlive(alive), log(l) {}

    void update(float) override {
      log->push_back("update:" + name);
      if (name == "dead-after-update") isAlive = false;
    }

    void render(SDL_Renderer*) override { log->push_back("render:" + name); }

    int zIndex() const override { return z; }
    bool alive() const override { return isAlive; }
  };

  std::vector<std::string> log;
  Scene scene;

  // Insert order is A then B, but B has lower z so it must render first.
  scene.add(std::make_unique<TraceObject>("A", 10, &log));
  scene.add(std::make_unique<TraceObject>("B", 0, &log));
  scene.add(std::make_unique<TraceObject>("dead-after-update", 5, &log));

  scene.update(0.016f);
  scene.render(nullptr);

  // Update is called in insertion order.
  assert((log.size() >= 3));
  assert(log[0] == "update:A");
  assert(log[1] == "update:B");
  assert(log[2] == "update:dead-after-update");

  // Dead object removed before rendering.
  // Render order sorted by zIndex: B (0) then A (10).
  bool sawRenderB = false, sawRenderA = false;
  for (const auto& s : log) {
    if (s == "render:B") sawRenderB = true;
    if (s == "render:A") sawRenderA = true;
    assert(s != "render:dead-after-update");
  }
  assert(sawRenderB && sawRenderA);

  // Ensure ordering among renders: B before A.
  auto pos = [&](const std::string& needle) -> int {
    for (int i = 0; i < static_cast<int>(log.size()); ++i)
      if (log[i] == needle) return i;
    return -1;
  };
  const int rb = pos("render:B");
  const int ra = pos("render:A");
  assert(rb != -1 && ra != -1);
  assert(rb < ra);
}

int main() {
  testMoveLeftSimpleMerge();
  testMoveLeftMergeOnceRule();
  testMoveRightDoubleMerge();
  testMoveUpColumnMerge();
  testGameOverDetection();
  testSpawnPendingAndCommit();
  testIntegrationSceneLifecycleAndOrdering();
  std::cout << "All tests passed.\n";
  return 0;
}


