#pragma once

#include <bus.h>

#include <cstdint>
#include <vector>

namespace gba
{
  /// PPU is responsible for reading the first 76000 bytes of VRAM from the Bus
  /// and rendering the current frame to the screen.
  /// It decodes the 16-bit RGB555 colours into standard 32-bit RGBA8888 format for display.
  class PPU {
  public:
    explicit PPU(Bus& bus_);

    ~PPU() = default;

    /// renderFrame() renders a single frame by reading from VRAM and updating the display.
    void renderFrame();

    [[nodiscard]] const uint32_t* getFrameBuffer() const;

    // GBA Screen Dimensions
    static constexpr int SCREEN_WIDTH = 240;
    static constexpr int SCREEN_HEIGHT = 160;

  private:
    void renderMode3();

    Bus& bus;

    // Frame buffer to hold pixel data for the current frame
    // 240 x 160 array of 32-bit pixels (RGBA8888)
    std::vector<uint32_t> frameBuffer;
  };
} // namespace gba
