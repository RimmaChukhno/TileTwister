#pragma once

#include <SDL2/SDL.h>

#include <string>

class Window {
public:
  Window() = default;
  ~Window();

  bool init(const std::string& title, int w, int h);
  void shutdown();

  SDL_Window* sdlWindow() const { return m_window; }
  SDL_Renderer* renderer() const { return m_renderer; }
  int width() const { return m_w; }
  int height() const { return m_h; }

private:
  SDL_Window* m_window = nullptr;
  SDL_Renderer* m_renderer = nullptr;
  int m_w = 0;
  int m_h = 0;
};


