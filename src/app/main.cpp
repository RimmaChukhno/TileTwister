#include <tiletwister/app/GameControllerObject.hpp>
#include <tiletwister/engine/Scene.hpp>
#include <tiletwister/platform/Window.hpp>

#include <SDL2/SDL.h>

#include <cstdint>
#include <memory>

int main(int, char**) {
  Window win;
  if (!win.init("Tile Twister 2048", 600, 600)) {
    return 1;
  }

  bool running = true;
  Uint64 last = SDL_GetPerformanceCounter();

  Scene scene;
  auto controller = std::make_unique<GameControllerObject>(&running);
  controller->setWindowSize(win.width(), win.height());
  scene.add(std::move(controller));

  while (running) {
    // Timing
    const Uint64 now = SDL_GetPerformanceCounter();
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
    const float dt = static_cast<float>((now - last) / freq);
    last = now;

    // Input
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
      } else {
        scene.handleEvent(e);
      }
    }

    scene.update(dt);
    scene.render(win.renderer());
  }

  win.shutdown();
  return 0;
}


