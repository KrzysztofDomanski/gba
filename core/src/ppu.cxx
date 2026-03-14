#include <ppu.h>

using namespace gba;

PPU::PPU(Bus& bus_) : bus(bus_), frameBuffer(SCREEN_WIDTH * SCREEN_HEIGHT, 0xFF000000)
{
}

const uint32_t* PPU::getFrameBuffer() const
{
  return frameBuffer.data();
}

void PPU::renderFrame()
{
  // TODO A real GBA checks memory address 0x04000000 (DISPCNT) to see what mode it's in
  // For now we assume mode 3 so that we can see SOMETHING on the screen
  renderMode3();
}

void PPU::renderMode3()
{
  // Mode 3 is a simple bitmap mode where each pixel is represented by a 16-bit RGB555 value in VRAM.
  // The GBA screen is 240x160 pixels, so we need to read the first 76000 bytes of VRAM (240 * 160 * 2).
  for (int y = 0; y < SCREEN_HEIGHT; ++y) {
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
      uint32_t vramAddress = (y * SCREEN_WIDTH + x) * 2;     // Each pixel is 2 bytes (1 halfword)
      uint16_t color = bus.read16(0x06000000 + vramAddress); // Read from VRAM

      // Decode RGB555 to RGBA8888
      uint8_t r = (color & 0x1F) << 3;         // Red is bits [4:0]
      uint8_t g = ((color >> 5) & 0x1F) << 3;  // Green is bits [9:5]
      uint8_t b = ((color >> 10) & 0x1F) << 3; // Blue is bits [14:10]

      // We use standard RGBA here but on Apple Sillicon and Windows we might want BGRA instead
      // That can be sorted out in the UI layer later
      uint32_t rgba = (r << 24) | (g << 16) | (b << 8) | 0xFF;

      frameBuffer[y * SCREEN_WIDTH + x] = rgba; // Alpha is always 255
    }
  }
}
