#include "Window.hpp"

Window::Window(int w, int h, const char* title) : m_w(w), m_h(h) {
    SDL_Init(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, w, h, 0);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
}

Window::~Window() {
    if(m_renderer) SDL_DestroyRenderer(m_renderer);
    if(m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
}

int Window::width() const { return m_w; }
int Window::height() const { return m_h; }
