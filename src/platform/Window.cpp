#include <tiletwister/platform/Window.h>

Window::~Window() { shutdown(); }

bool Window::init(const std::string& title, int w, int h) {
  m_w = w;
  m_h = h;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    return false;
  }

  m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
  if (!m_window) {
    shutdown();
    return false;
  }

  m_renderer = SDL_CreateRenderer(
      m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!m_renderer) {
    shutdown();
    return false;
  }

  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  return true;
}

void Window::shutdown() {
  if (m_renderer) {
    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;
  }
  if (m_window) {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }
  SDL_Quit();
}


