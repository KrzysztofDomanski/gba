#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU Thumb Format 9 (Load/Store)", "[cpu][thumb][lsu]")
{
  gba::Bus bus;
  std::filesystem::path temp_rom_path = "test_thumb_lsu.gba";

  std::vector<uint32_t> instructions = {
      0xE3A01403, // ARM: MOV R1, #0x03000000
      0xE28F0001, // ARM: ADD R0, PC, #1
      0xE12FFF10, // ARM: BX R0
      0x604A222A, // Packed THUMB: STR R2, [R1, #4] (Upper) | MOV R2, #42 (Lower)
      0x0000684B  // Packed THUMB: NOP (Upper) | LDR R3, [R1, #4] (Lower)
  };

  {
    std::ofstream file(temp_rom_path, std::ios::binary);
    // Correctly using sizeof(uint32_t)!
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(temp_rom_path) == true);
  gba::CPU cpu(bus);

  SECTION("Execute THUMB STR and LDR")
  {
    cpu.reset();

    cpu.step(); // MOV R1, #0x03000000
    cpu.step(); // ADD R0, PC, #1
    cpu.step(); // BX R0 - should switch to THUMB state and jump to 0x08000008

    REQUIRE((cpu.getCPSR() & 0x20) != 0);      // Check THUMB state bit is set
    REQUIRE(cpu.getRegister(1) == 0x03000000); // R1 should be 0x03000000

    // THUMB: MOV R2, #42
    cpu.step();
    REQUIRE(cpu.getRegister(2) == 42);

    // STR R2, [R1, #4] - should store 42 at address 0x03000004
    cpu.step();
    REQUIRE(bus.read32(0x03000004) == 42); // Verify the value was stored in memory

    // LDR R3, [R1, #4] - should load the value at 0x03000004 into R3
    cpu.step();
    REQUIRE(cpu.getRegister(3) == 42); // R3 should be
  }

  std::filesystem::remove(temp_rom_path);
}