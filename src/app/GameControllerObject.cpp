#include <tiletwister/app/GameControllerObject.hpp>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>

GameControllerObject::GameControllerObject(bool *runningFlag)
    : m_running(runningFlag)
{
  loadScores();
  rebuildTilesFromGrid();
}

void GameControllerObject::loadScores()
{
  // Format (simple + human-editable):
  // best=<int>
  // last=<int>
  //
  // Backward compatible: if file contains just 1 or 2 integers, treat them as
  // best then last.
  std::ifstream in(m_scoresPath);
  if (!in.is_open())
    return;

  int best = 0;
  int last = 0;
  bool bestSet = false;
  bool lastSet = false;

  std::string line;
  while (std::getline(in, line))
  {
    // Trim
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
      line.pop_back();
    if (line.empty())
      continue;

    auto parseInt = [](const std::string &s, int &out) -> bool
    {
      std::istringstream ss(s);
      ss >> out;
      return !ss.fail();
    };

    const std::string bestPrefix = "best=";
    const std::string lastPrefix = "last=";
    if (line.rfind(bestPrefix, 0) == 0)
    {
      int v = 0;
      if (parseInt(line.substr(bestPrefix.size()), v))
      {
        best = std::max(0, v);
        bestSet = true;
      }
      continue;
    }
    if (line.rfind(lastPrefix, 0) == 0)
    {
      int v = 0;
      if (parseInt(line.substr(lastPrefix.size()), v))
      {
        last = std::max(0, v);
        lastSet = true;
      }
      continue;
    }

    // Fallback: plain integer lines
    int v = 0;
    if (parseInt(line, v))
    {
      if (!bestSet)
      {
        best = std::max(0, v);
        bestSet = true;
      }
      else if (!lastSet)
      {
        last = std::max(0, v);
        lastSet = true;
      }
    }
  }

  m_bestScore = best;
  m_lastScore = last;
  m_savedBestScore = m_bestScore;
  m_savedLastScore = m_lastScore;
}

void GameControllerObject::saveScoresIfNeeded(bool force)
{
  const int best = std::max(0, m_bestScore);
  const int last = std::max(0, m_lastScore);

  if (!force && best == m_savedBestScore && last == m_savedLastScore)
    return;

  std::ofstream out(m_scoresPath, std::ios::trunc);
  if (!out.is_open())
    return;

  out << "best=" << best << "\n";
  out << "last=" << last << "\n";

  m_savedBestScore = best;
  m_savedLastScore = last;
}

int GameControllerObject::keyForCellValue(int r, int c, int value, int ordinal)
{
  return (r & 0xF) | ((c & 0xF) << 4) | ((value & 0xFFFF) << 8) |
         ((ordinal & 0xFF) << 24);
}

void GameControllerObject::rebuildTilesFromGrid()
{
  m_tiles.clear();
  int counts[16]{};
  for (int r = 0; r < 4; ++r)
  {
    for (int c = 0; c < 4; ++c)
    {
      const int v = m_game.grid()[r][c];
      if (v == 0)
        continue;
      const int idx = r * 4 + c;
      const int ord = counts[idx]++;
      const int k = keyForCellValue(r, c, v, ord);
      m_tiles.emplace(k, Tile(v, Cell{r, c}));
    }
  }
}

void GameControllerObject::beginMove(Direction dir)
{
  if (m_activeMove.active)
    return;

  int before[4][4]{};
  std::memcpy(before, m_game.grid(), sizeof(before));

  const MoveResult mr = m_game.tryMove(dir);
  if (!mr.moved)
    return;

  if (m_game.score() > m_bestScore)
  {
    m_bestScore = m_game.score();
    saveScoresIfNeeded(false);
  }

  m_tiles.clear();

  // Build tiles from animation sources.
  int srcSeen[4][4]{};
  for (const auto &a : mr.animations)
  {
    const int ord = srcSeen[a.from.r][a.from.c]++;
    const int k = keyForCellValue(a.from.r, a.from.c, a.value, ord);
    Tile t(a.value, a.from);
    t.startSlide(a.from, a.to, m_activeMove.duration);
    m_tiles.emplace(k, t);
  }

  // Add stationary tiles (ones not referenced as a move source).
  bool usedFrom[4][4]{};
  for (const auto &a : mr.animations)
    usedFrom[a.from.r][a.from.c] = true;
  for (int r = 0; r < 4; ++r)
  {
    for (int c = 0; c < 4; ++c)
    {
      const int v = before[r][c];
      if (v == 0)
        continue;
      if (usedFrom[r][c])
        continue;
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

void GameControllerObject::handleEvent(const SDL_Event &e)
{
  // Game-over modal: mouse hover + click to restart.
  if (m_game.isGameOver())
  {
    if (e.type == SDL_MOUSEMOTION)
    {
      const SDL_Rect btn =
          Renderer::computeGameOverButtonRect(m_windowW, m_windowH);
      const int mx = e.motion.x;
      const int my = e.motion.y;
      m_gameOverButtonHover =
          (mx >= btn.x && mx < btn.x + btn.w && my >= btn.y &&
           my < btn.y + btn.h);
      return;
    }
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
    {
      const SDL_Rect btn =
          Renderer::computeGameOverButtonRect(m_windowW, m_windowH);
      const int mx = e.button.x;
      const int my = e.button.y;
      const bool inside =
          (mx >= btn.x && mx < btn.x + btn.w && my >= btn.y &&
           my < btn.y + btn.h);
      if (inside)
      {
        // Save last run score + possibly update best, then reset.
        m_lastScore = std::max(0, m_game.score());
        if (m_lastScore > m_bestScore)
          m_bestScore = m_lastScore;
        saveScoresIfNeeded(true);

        m_game.reset();
        m_activeMove.active = false;
        m_game.commitPendingSpawn();
        rebuildTilesFromGrid();
        m_gameOverButtonHover = false;
      }
      return;
    }
  }

  if (e.type != SDL_KEYDOWN)
    return;

  const SDL_Keycode key = e.key.keysym.sym;
  if (key == SDLK_ESCAPE)
  {
    // Persist best/last on quit.
    saveScoresIfNeeded(true);
    if (m_running)
      *m_running = false;
    return;
  }

  if (key == SDLK_r)
  {
    // Save last run score + possibly update best, then reset.
    m_lastScore = std::max(0, m_game.score());
    if (m_lastScore > m_bestScore)
      m_bestScore = m_lastScore;
    saveScoresIfNeeded(true);

    m_game.reset();
    // Keep best score across resets.
    m_activeMove.active = false;
    m_game.commitPendingSpawn();
    rebuildTilesFromGrid();
    m_gameOverButtonHover = false;
    return;
  }

  if (m_activeMove.active)
    return;

  if (key == SDLK_LEFT)
    beginMove(Direction::Left);
  else if (key == SDLK_RIGHT)
    beginMove(Direction::Right);
  else if (key == SDLK_UP)
    beginMove(Direction::Up);
  else if (key == SDLK_DOWN)
    beginMove(Direction::Down);
}

void GameControllerObject::update(float dtSec)
{
  for (auto &kv : m_tiles)
    kv.second.update(dtSec);

  if (m_game.score() > m_bestScore)
  {
    m_bestScore = m_game.score();
    saveScoresIfNeeded(false);
  }

  if (!m_activeMove.active)
    return;

  m_activeMove.timeLeft -= dtSec;
  if (m_activeMove.timeLeft > 0.0f)
    return;

  m_activeMove.active = false;

  // Commit spawn and rebuild final board tiles.
  m_game.commitPendingSpawn();
  rebuildTilesFromGrid();

  // Pop merged destinations.
  for (const Cell &c : m_activeMove.pendingPopCells)
  {
    for (auto &kv : m_tiles)
    {
      if (kv.second.cell().r == c.r && kv.second.cell().c == c.c)
      {
        kv.second.startPop(0.10f);
      }
    }
  }

  // Pop newly spawned tile (if any).
  if (m_activeMove.pendingSpawnCell.has_value())
  {
    const Cell c = *m_activeMove.pendingSpawnCell;
    for (auto &kv : m_tiles)
    {
      if (kv.second.cell().r == c.r && kv.second.cell().c == c.c)
      {
        kv.second.startPop(0.12f);
      }
    }
  }

  m_activeMove.pendingPopCells.clear();
  m_activeMove.pendingSpawnCell.reset();

  // If the move ended (spawn committed) and score just increased, persist.
  saveScoresIfNeeded(false);
}

void GameControllerObject::render(SDL_Renderer *renderer)
{
  m_renderer.render(renderer, m_game, m_tiles, m_windowW, m_windowH,
                    m_game.score(), m_bestScore, m_game.isGameOver(),
                    m_gameOverButtonHover);
}
