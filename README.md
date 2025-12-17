# Tile Twister (2048) — C++ / SDL2

Small 2048-inspired puzzle game in **C++** using **SDL2** (pink background, animated tiles).

## Build (Windows / MSYS2)

This repo is set up for **MSYS2 MinGW64** installed at `C:\msys64\...`.

- **Build the game (Makefile)**:

  - From PowerShell:
    - `C:\msys64\usr\bin\make.exe`
    - `.\build\main.exe`

- **Run tests (logic-only, Makefile)**:
  - From PowerShell:
    - `C:\msys64\usr\bin\make.exe test`
    - `.\build\tests.exe`

- **Build with CMake (recommended for IDEs)**:
  - Configure:
    - `cmake -S . -B build/cmake -G "MinGW Makefiles"`
  - Build:
    - `cmake --build build/cmake`
  - Run:
    - `.\build\cmake\tiletwister.exe`
    - `.\build\cmake\tiletwister_tests.exe`

## Controls

- **Arrow keys**: move tiles
- **R**: restart
- **ESC**: quit

## Project structure

- `include/tiletwister/**`: public headers
- `src/**`: implementation
- `tests/**`: logic tests (no SDL window)

## Start

mingw32-make
.\build\main.exe

