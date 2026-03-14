#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU to VRAM Pixel Drawing", "[bus][vram]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "dummy_cartridge.gba";

  // Assembly Program:
  // 1. MOV R0, #0x06000000 -> Set R0 to the start of VRAM
  //    Hex: 0xE3A00406
  // 2. MOV R1, #0x1F     -> Set R1 to a color value
  //    Hex: 0xE3A0101F
  // 3. STR R1, [R0]      -> Store the color value at the VRAM address
  //    Hex: 0xE1C010B0

  std::vector<uint32_t> instructions = {
      0xE3A00406, // MOV R0, #0x06000000
      0xE3A0101F, // MOV R1, #0x1F
      0xE1C010B0  // STR R1, [R0]
  };

  {
    // Create a dummy cartridge file with the test program
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute STRH to VRAM and verify mirroring")
  {
    cpu.reset();

    cpu.step(); // MOV R0, #0x06000000
    cpu.step(); // MOV R1, #0x1F
    cpu.step(); // STR R1, [R0]

    // Verify the value was written to VRAM
    REQUIRE(bus.read16(0x06000000) == 0x001F);

    bus.write16(0x06018000, 0x7FFF);           // Write to the mirrored address
    REQUIRE(bus.read16(0x06010000) == 0x7FFF); // Should read the mirrored value
  }

  std::filesystem::remove(testGbaPath);
}
