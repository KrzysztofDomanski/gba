#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU THUMB Conditional Branch (Loop)", "[thumb][branch]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "thumb_branch_test.gba";

  // Assembly Program:
  // --- ARM STATE ---
  // 0x08000000: MOV R0, #0x08000009  (Target is 0x08000008 + 1 for THUMB) -> 0xE3A00009
  // 0x08000004: BX R0                (Jump and switch state)              -> 0xE12FFF10
  // --- THUMB STATE ---
  // 0x08000008: MOV R1, #3           (0x2103)
  // 0x0800000A: SUB R1, #1           (0x3901) <-- LOOP START
  // 0x0800000C: BNE 0x0800000A       (0xD1FD) (Target 0x0A - PC 0x10 = -6 bytes = -3 halfwords = 0xFD)

  std::vector<uint32_t> instructions = {
      0xE28F0001, // ADD R0, PC, #1
      0xE12FFF10, // BX R0
      0x39012103, // Packed: MOV R1, #3 (Lower) | SUB R1, #1 (Upper)
      0x0000D1FD  // Packed: BNE (Lower) | NOP (Upper)
  };

  {
    // Create a dummy cartridge file with the test program
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute State Switch and THUMB Countdown")
  {
    cpu.reset();

    cpu.step(); // MOV R0, #0x08000009
    cpu.step(); // BX R0 - should switch to THUMB state and jump to 0x08000008

    REQUIRE((cpu.getCPSR() & 0x20) != 0); // Check THUMB state bit is set

    cpu.step();                       // THUMB: MOV R1, #3
    REQUIRE(cpu.getRegister(1) == 3); // R1 should be 3

    // Iteration 1
    cpu.step();                       // THUMB: SUB R1, #1
    cpu.step();                       // THUMB: BNE (should branch back to SUB)
    REQUIRE(cpu.getRegister(1) == 2); // R1 should be 2

    // Iteration 2
    cpu.step();                       // THUMB: SUB R1, #1
    cpu.step();                       // THUMB: BNE (should branch back to SUB)
    REQUIRE(cpu.getRegister(1) == 1); // R1 should be 1

    // Iteration 3
    cpu.step();                       // THUMB: SUB R1, #1
    cpu.step();                       // THUMB: BNE (should NOT branch this time)
    REQUIRE(cpu.getRegister(1) == 0); // R1 should be 0

    REQUIRE(cpu.getRegister(15) == 0x08000012);
  }

  std::filesystem::remove(testGbaPath);
}