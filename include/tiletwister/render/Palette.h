#pragma once

#include <SDL2/SDL.h>

namespace Palette {

SDL_Color backgroundPink(); // RGB(255, 182, 193)
SDL_Color tileColor(int value);
SDL_Color tileBorderColor();
SDL_Color gridEmptyCellColor();

} // namespace Palette


