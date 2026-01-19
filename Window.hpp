#pragma once
#include <SDL.h>

class Window {
public:
    Window(int w, int h, const char* title);
    ~Window();
    SDL_Renderer* renderer() const { return m_renderer; }
    int width() const;
    int height() const;

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    int m_w, m_h;
};
