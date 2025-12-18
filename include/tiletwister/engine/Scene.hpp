#pragma once

#include <tiletwister/engine/GameObject.hpp>

#include <algorithm>
#include <memory>
#include <vector>

class Scene {
public:
  void add(std::unique_ptr<GameObject> obj) { m_objects.emplace_back(std::move(obj)); }

  void handleEvent(const SDL_Event& e) {
    for (auto& o : m_objects) o->handleEvent(e);
  }

  void update(float dtSec) {
    for (auto& o : m_objects) o->update(dtSec);
    // Remove dead objects
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
                                   [](const std::unique_ptr<GameObject>& o) {
                                     return !o->alive();
                                   }),
                    m_objects.end());
  }

  void render(SDL_Renderer* r) {
    // Stable sort by zIndex each frame (small N for this project).
    std::stable_sort(m_objects.begin(), m_objects.end(),
                     [](const std::unique_ptr<GameObject>& a,
                        const std::unique_ptr<GameObject>& b) {
                       return a->zIndex() < b->zIndex();
                     });
    for (auto& o : m_objects) o->render(r);
  }

private:
  std::vector<std::unique_ptr<GameObject>> m_objects;
};


