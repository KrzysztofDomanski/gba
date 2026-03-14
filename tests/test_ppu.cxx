#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <ppu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("PPU Mode 3 RGB555 to RGBA Conversion", "[ppu][vram]")
{
  gba::Bus bus;
  gba::PPU ppu(bus);

  SECTION("Render a single pixel in Mode 3")
  {
    int targetX = 10;
    int targetY = 5;

    // Find the VRAM address for the target pixel (x, y)
    uint32_t vramAddress = 0x06000000 + (targetY * gba::PPU::SCREEN_WIDTH + targetX) * 2;

    // Pure Blue in GBA RGB555 format.
    // Red = 0, Green = 0, Blue = 31 (11111 in binary)
    // Shift Blue left by 10. (31 << 10) = 0x7C00
    bus.write16(vramAddress, 0x7C00);

    ppu.renderFrame();

    const uint32_t* frameBuffer = ppu.getFrameBuffer();
    uint32_t pixelColor = frameBuffer[targetY * gba::PPU::SCREEN_WIDTH + targetX];

    REQUIRE(pixelColor == 0x0000F8FF);
  }
}