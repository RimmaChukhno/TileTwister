#pragma once

#include <tiletwister/engine/GameObject.hpp>

#include <tiletwister/game/Game.hpp>
#include <tiletwister/game/Tile.hpp>
#include <tiletwister/render/Renderer.hpp>

#include <optional>
#include <unordered_map>
#include <vector>

class GameControllerObject final : public GameObject {
public:
  explicit GameControllerObject(bool* runningFlag);

  void setWindowSize(int w, int h) {
    m_windowW = w;
    m_windowH = h;
  }

  void handleEvent(const SDL_Event& e) override;
  void update(float dtSec) override;
  void render(SDL_Renderer* renderer) override;

private:
  struct ActiveMove {
    bool active = false;
    float timeLeft = 0.0f;
    float duration = 0.12f;
    std::vector<Cell> pendingPopCells;
    std::optional<Cell> pendingSpawnCell;
  };

  bool* m_running = nullptr;
  int m_windowW = 600;
  int m_windowH = 600;

  Game m_game;
  Renderer m_renderer;
  std::unordered_map<int, Tile> m_tiles;
  ActiveMove m_activeMove{};

  void rebuildTilesFromGrid();
  void beginMove(Direction dir);

  static int keyForCellValue(int r, int c, int value, int ordinal);
};


