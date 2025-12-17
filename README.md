# Tile Twister (2048) — C++ / SDL2

Small 2048-inspired puzzle game in **C++** using **SDL2** (pink background, animated tiles).

## Build (Windows / MSYS2)

This repo is set up for **MSYS2 MinGW64** installed at `C:\msys64\...`.

- **Build the game**:

  - From PowerShell:
    - `C:\msys64\usr\bin\make.exe`

- **Run tests (logic-only)**:
  - From PowerShell:
    - `C:\msys64\usr\bin\make.exe test`
    - `.\tests.exe`

## Controls

- **Arrow keys**: move tiles
- **R**: restart
- **ESC**: quit

## Project structure

- `main.cpp`: entry point + main loop
- `Game.h/.cpp`: 2048 logic (moves, merges, spawn, game-over)
- `Tile.h/.cpp`: tile animation (slide + merge/spawn pop)
- `Window.h/.cpp`: SDL2 init + window/renderer creation
- `Renderer.h/.cpp`: drawing (board, tiles, value text)
- `Utils.h/.cpp`: RNG, easing, colors
- `tests.cpp`: integration-ish tests for logic (no SDL window)


## Lancer

mingw32-make
.\main.exe

