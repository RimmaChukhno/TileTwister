cmake_minimum_required(VERSION 3.16)
project(TileTwister2048 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Core / Game libraries (no SDL) ----
add_library(tiletwister_core
  src/core/Utils.cpp
)
target_include_directories(tiletwister_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tiletwister_game
  src/game/Game.cpp
  src/game/Tile.cpp
)
target_include_directories(tiletwister_game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tiletwister_game PUBLIC tiletwister_core)

# ---- SDL2 ----
# Prefer CONFIG package (common on MSYS2), fallback to classic FindSDL2.
find_package(SDL2 CONFIG QUIET)
if (SDL2_FOUND)
  # MinGW linkers are order-sensitive: SDL2main must come before SDL2.
  set(TILETWISTER_SDL_LIBS SDL2::SDL2main SDL2::SDL2)
else()
  find_package(SDL2 REQUIRED)
  set(TILETWISTER_SDL_LIBS ${SDL2_LIBRARIES})
endif()

# ---- Render / Platform libraries (SDL) ----
add_library(tiletwister_render
  src/render/Palette.cpp
  src/render/Renderer.cpp
)
target_include_directories(tiletwister_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tiletwister_platform
  src/platform/Window.cpp
)
target_include_directories(tiletwister_platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---- App ----
add_executable(tiletwister
  src/app/main.cpp
  src/app/GameControllerObject.cpp
)
target_include_directories(tiletwister PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tiletwister PRIVATE tiletwister_game tiletwister_render tiletwister_platform ${TILETWISTER_SDL_LIBS})

if (MINGW)
  target_link_options(tiletwister PRIVATE -mwindows)
endif()

# ---- Tests (no SDL) ----
add_executable(tiletwister_tests
  tests/tests.cpp
)
target_include_directories(tiletwister_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tiletwister_tests PRIVATE tiletwister_game)


