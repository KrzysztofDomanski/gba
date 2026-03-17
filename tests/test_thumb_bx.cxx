#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU THUMB Format 5 (BX back to ARM)", "[cpu][thumb][bx]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "dummy_cartridge.gba";

  std::vector<uint32_t> instructions = {
      0xE28F8008, // ARM: ADD R8, PC, #8 (R8 = 0x08000010)
      0xE28F0001, // ARM: ADD R0, PC, #1
      0xE12FFF10, // ARM: BX R0
      0x47402163, // Packed THUMB: BX R8 (Upper) | MOV R1, #99 (Lower)
      0xE3A0202A  // ARM: MOV R2, #42
  };

  {
    // Create a dummy cartridge file with the test program
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute Full Architecture Round-Trip")
  {
    cpu.reset();

    cpu.step(); // Execute MOV R8, #0x08000010
    cpu.step(); // Execute ADD R0, PC, #1
    cpu.step(); // Execute BX R0 (Jump to 0x08000010)

    REQUIRE((cpu.getCPSR() & 0x20) != 0); // Should be in THUMB state

    cpu.step(); // THUMB: MOV R1, #99
    REQUIRE(cpu.getRegister(1) == 99);

    cpu.step();                           // Execute BX R8 (Jump back to ARM state at 0x08000010)
    REQUIRE((cpu.getCPSR() & 0x20) == 0); // Should be back in ARM state

    // Verify pipeline jumped corretly
    REQUIRE(cpu.getRegister(15) == 0x08000018);

    cpu.step(); // Execute MOV R2, #42
    REQUIRE(cpu.getRegister(2) == 42);
  }

  std::filesystem::remove(testGbaPath);
}