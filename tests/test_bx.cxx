#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU Branch and Exchange State Switch", "[cpu][bx][thumb]")
{
  gba::Bus bus;
  std::filesystem::path testRomPath = "bx_test.gba";

  // Assembly Program:
  // BX R0 Hex: 0xE12FFF10 (Cond: AL, Op: BX, Rn: 0)

  std::vector<uint32_t> instructions = {
      0xE12FFF10, // BX R0
      0x00000000, // NOP (will be executed in THUMB mode)
      0x00000000, // NOP (will be executed in THUMB mode)
      0x00000000, // NOP (will be executed in THUMB mode)
      0x00000000, // NOP (will be executed in THUMB mode)
                  // Thumb code would start here
  };

  {
    std::ofstream file(testRomPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testRomPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute BX and switch to THUMB state")
  {
    cpu.reset();

    cpu.setRegister(0, 0x08000011);

    // Initially in ARM state - the T bit is 0
    REQUIRE((cpu.getCPSR() & 0x20) == 0);

    // Execute the BX instruction
    cpu.step();

    // PC should be set to 0x08000014 (R0 with bit 0 cleared)
    REQUIRE(cpu.getRegister(15) == 0x08000014);

    // T bit should be set, indicating THUMB state
    REQUIRE((cpu.getCPSR() & 0x20) == 0x20);
  }

  std::filesystem::remove(testRomPath);
}
