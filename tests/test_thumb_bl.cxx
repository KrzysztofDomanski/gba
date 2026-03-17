#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU THUMB Format 19 (Branch with Link)", "[cpu][thumb][branch]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "dummy_cartridge.gba";

  std::vector<uint32_t> instructions = {
      0xE28F0001, // ARM: ADD R0, PC, #1
      0xE12FFF10, // ARM: BX R0
      0xF804F000, // Packed THUMB: Suffix (Upper) | Prefix (Lower)
      0x000046C0, // Packed THUMB: NOP (Upper) | NOP (Lower)
      0x00000000, // Empty space (Addresses 0x10 to 0x13)
      0x0000212A  // Packed THUMB: NOP (Upper) | MOV R1, #42 (Lower) at 0x14
  };

  {
    // Create a dummy cartridge file with the test program
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute THUMB BL instruction")
  {
    cpu.reset();

    cpu.step(); // Execute ADD R0, PC, #1
    cpu.step(); // Execute BX R0 (Jump to 0x14)
    cpu.step(); // Execute BL Prefix (sets LR to 0x18)
    cpu.step(); // Execute BL Suffix (jumps to 0x14)

    // Because of the pipeline flush, the PC will read 4 bytes ahead of the executing instruction.
    // Target is 0x14, so PC should be 0x18.
    REQUIRE(cpu.getRegister(15) == 0x08000018);

    // The instruction after the BL suffix is at 0x0800000C.
    // Bit 0 must be set to 1 to indicate THUMB state, so 0x0C | 1 = 0x0D.
    REQUIRE(cpu.getRegister(14) == 0x0800000D);

    cpu.step(); // THUMB: MOV R1, #42
    REQUIRE(cpu.getRegister(1) == 42);
  }

  std::filesystem::remove(testGbaPath);
}