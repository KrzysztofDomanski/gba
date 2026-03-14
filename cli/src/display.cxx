#include <display.h>

using namespace gba::cli;

Display::Display() : window(nullptr), renderer(nullptr), texture(nullptr)
{
}

Display::~Display()
{
  if (texture) {
    SDL_DestroyTexture(texture);
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
}

bool Display::initialize()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return false;
  }

  window = SDL_CreateWindow("GBA Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * SCALE,
                            HEIGHT * SCALE, SDL_WINDOW_SHOWN);
  if (!window) {
    SDL_Log("Failed to create window: %s", SDL_GetError());
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    SDL_Log("Failed to create renderer: %s", SDL_GetError());
    return false;
  }

  // Texture format matches the output of the PPU's frame buffer (RGBA8888)
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  if (!texture) {
    SDL_Log("Failed to create texture: %s", SDL_GetError());
    return false;
  }

  return true;
}

bool Display::processEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      return false; // Signal to exit the main loop
    }
  }
  return true;
}

void Display::render(const uint32_t* frameBuffer)
{
  // Update the texture with the latest frame buffer from the PPU
  SDL_UpdateTexture(texture, nullptr, frameBuffer, WIDTH * 4);

  // Clear the renderer and copy the texture to it
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}
