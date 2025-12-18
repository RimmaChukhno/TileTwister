#pragma once

#include <SDL2/SDL.h>

class GameObject {
public:
  virtual ~GameObject() = default;

  // Optional: input
  virtual void handleEvent(const SDL_Event&) {}

  // Required: update & draw
  virtual void update(float dtSec) = 0;
  virtual void render(SDL_Renderer* renderer) = 0;

  // Optional: ordering / lifetime
  virtual int zIndex() const { return 0; }
  virtual bool alive() const { return true; }
};


