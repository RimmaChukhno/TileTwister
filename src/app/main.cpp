#include <tiletwister/game/Game.h>
#include <tiletwister/game/Tile.h>
#include <tiletwister/platform/Window.h>
#include <tiletwister/render/Renderer.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <unordered_map>
#include <vector>

namespace {

struct ActiveMove {
  bool active = false;
  float timeLeft = 0.0f;
  float duration = 0.12f;
  std::vector<Cell> pendingPopCells;
  std::optional<Cell> pendingSpawnCell;
};

int keyForCellValue(int r, int c, int value, int ordinal) {
  // Stable-ish key to track visual tiles across frames.
  // Because merges create ambiguity, we use an ordinal for duplicates.
  // Key space: (r,c,value,ordinal) packed.
  return (r & 0xF) | ((c & 0xF) << 4) | ((value & 0xFFFF) << 8) |
         ((ordinal & 0xFF) << 24);
}

} // namespace

int main(int, char**) {
  Window win;
  if (!win.init("Tile Twister 2048", 600, 600)) {
    return 1;
  }

  Game game;
  Renderer renderer;

  std::unordered_map<int, Tile> tiles;

  auto rebuildTilesFromGrid = [&]() {
    tiles.clear();
    int counts[16]{};
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        const int v = game.grid()[r][c];
        if (v == 0) continue;
        const int idx = r * 4 + c;
        const int ord = counts[idx]++;
        const int k = keyForCellValue(r, c, v, ord);
        tiles.emplace(k, Tile(v, Cell{r, c}));
      }
    }
  };

  rebuildTilesFromGrid();

  bool running = true;
  Uint64 last = SDL_GetPerformanceCounter();
  ActiveMove activeMove{};

  auto beginMove = [&](Direction dir) {
    if (activeMove.active) return;
    int before[4][4]{};
    std::memcpy(before, game.grid(), sizeof(before));
    const MoveResult mr = game.tryMove(dir);
    if (!mr.moved) return;

    tiles.clear();

    // Build tiles from animations sources (they represent the visual tiles).
    // Multiple tiles can target same cell (merge); we'll pop the destination.
    int srcSeen[4][4]{};
    for (const auto& a : mr.animations) {
      const int ord = srcSeen[a.from.r][a.from.c]++;
      const int k = keyForCellValue(a.from.r, a.from.c, a.value, ord);
      Tile t(a.value, a.from);
      t.startSlide(a.from, a.to, activeMove.duration);
      tiles.emplace(k, t);
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
        tiles.emplace(k, Tile(v, Cell{r, c}));
      }
    }

    activeMove.active = true;
    activeMove.timeLeft = activeMove.duration;

    activeMove.pendingPopCells = mr.mergedCells;
    if (mr.pendingSpawn.has_value())
      activeMove.pendingSpawnCell = mr.pendingSpawn->first;
    else
      activeMove.pendingSpawnCell.reset();
  };

  while (running) {
    // Timing
    const Uint64 now = SDL_GetPerformanceCounter();
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
    const float dt = static_cast<float>((now - last) / freq);
    last = now;

    // Input
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
      } else if (e.type == SDL_KEYDOWN) {
        const SDL_Keycode key = e.key.keysym.sym;
        if (key == SDLK_ESCAPE) {
          running = false;
        } else if (key == SDLK_r) {
          game.reset();
          activeMove.active = false;
          game.commitPendingSpawn();
          rebuildTilesFromGrid();
        } else if (!activeMove.active) {
          if (key == SDLK_LEFT) beginMove(Direction::Left);
          else if (key == SDLK_RIGHT) beginMove(Direction::Right);
          else if (key == SDLK_UP) beginMove(Direction::Up);
          else if (key == SDLK_DOWN) beginMove(Direction::Down);
        }
      }
    }

    // Update animations
    for (auto& kv : tiles) kv.second.update(dt);

    if (activeMove.active) {
      activeMove.timeLeft -= dt;
      if (activeMove.timeLeft <= 0.0f) {
        activeMove.active = false;
        // Commit spawn and rebuild final board tiles.
        game.commitPendingSpawn();
        rebuildTilesFromGrid();

        // Pop merged destinations.
        for (const Cell& c : activeMove.pendingPopCells) {
          for (auto& kv : tiles) {
            if (kv.second.cell().r == c.r && kv.second.cell().c == c.c) {
              kv.second.startPop(0.10f);
            }
          }
        }
        // Pop newly spawned tile (if any).
        if (activeMove.pendingSpawnCell.has_value()) {
          const Cell c = *activeMove.pendingSpawnCell;
          for (auto& kv : tiles) {
            if (kv.second.cell().r == c.r && kv.second.cell().c == c.c) {
              kv.second.startPop(0.12f);
            }
          }
        }
        activeMove.pendingPopCells.clear();
        activeMove.pendingSpawnCell.reset();
      }
    }

    // Render
    renderer.render(win.renderer(), game, tiles, win.width(), win.height(),
                    game.isGameOver());
  }

  win.shutdown();
  return 0;
}


