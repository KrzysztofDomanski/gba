#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>
#include <ppu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU to PPU Display Control", "[io][ppu]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "dummy_cartridge.gba";

  std::vector<uint32_t> instructions = {
      0xE3A00404, // MOV R0, #0x04000000
      0xE3A01003, // MOV R1, #3
      0xE3811B01, // ORR R1, R1, #0x0400  (R1 is now 0x0403)
      0xE1C010B0  // STRH R1, [R0]
  };

  {
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);
  gba::PPU ppu(bus);

  SECTION("Turn on screen and verify PPU state")
  {
    cpu.reset();

    // Before execution, DISPCNT is 0. PPU should output black even if VRAM has data.
    bus.write16(0x06000000, 0x7C00); // Write Pure Blue to VRAM
    ppu.renderFrame();
    REQUIRE(ppu.getFrameBuffer()[0] == 0xFF000000);

    // Execute the DISPCNT setup
    cpu.step(); // MOV R0
    cpu.step(); // MOV R1
    cpu.step(); // ORR R1
    cpu.step(); // STRH R1, [R0]

    // Verify the hardware register was actually written to
    REQUIRE(bus.read16(0x04000000) == 0x0403);

    // Tell the PPU to render again. It should now read DISPCNT,
    // realize Mode 3 is active, and draw our blue pixel!
    ppu.renderFrame();

    // Pure Blue (Red=0, Green=0, Blue=248, Alpha=255) -> 0x0000F8FF
    REQUIRE(ppu.getFrameBuffer()[0] == 0x0000F8FF);
  }

  std::filesystem::remove(testGbaPath);
}