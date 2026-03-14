#pragma once

// Stupid gotcha with SDL!! Took so long to figure this one out...
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <cstdint>

namespace gba::cli
{
  class Display {
  public:
    Display();
    ~Display();

    // initialize() initializes SDL, the window and the GBA Renderer
    [[nodiscard]] bool initialize();

    // processEvents() pumps OS events to keep the window responsive.
    [[nodiscard]] bool processEvents();

    // render() takes the current frame buffer from the PPU and renders it to the screen.
    void render(const uint32_t* frameBuffer);

  private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    // GBA Resolution
    static constexpr int WIDTH = 240;
    static constexpr int HEIGHT = 160;

    // Scale factor for better visibility on modern screens
    static constexpr int SCALE = 3;
  };
} // namespace gba::cli
