#pragma once

#include <SDL2/SDL.h>

class Score {
private:
    int m_score = 0;

public:
    void addScore(int points) { m_score += points; }
    int getScore() const { return m_score; }
    void reset() { m_score = 0; }

    void render(SDL_Renderer* r, int x, int y, int w, int h) const;
};