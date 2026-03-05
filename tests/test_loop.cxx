#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU Loop Execution (SUB, CMP, BNE)", "[cpu][loop]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "dummy_cartridge.gba";

  // Assembly Program: A simple countdown loop
  //
  // 0x08000000: MOV R0, #3       -> Initialize R0 to 3 (loop counter)
  //             Hex: 0xE3A00003
  //
  // 0x08000004: SUB R0, R0, #1   -> Decrement R0 by 1
  //             Hex: 0xE2400001
  //
  // 0x08000008: CMP R0, #0       -> Compare R0 to 0
  //             Hex: 0xE3500000
  // 0x0800000C: BNE 0x08000004   -> Branch to LOOP START if not equal (i.e., if R0 != 0) / Z = 0
  //             Offset calculation: Target(0x04) - PC(0x0c + 8) = -16 bytes = -4 words.
  //             -4 in 24-bit two's complement is 0xFFFFFC
  //             Format: Cond(NE:0001) | Fmt(101) | L(0) | Offset(FFFFFC)
  //             Hex: 0x1AFFFFFC

  std::vector<uint32_t> instructions = {
      0xE3A00003, // MOV R0, #3
      0xE2400001, // SUB R0, R0, #1
      0xE3500000, // CMP R0, #0
      0x1AFFFFFC  // BNE to SUB instruction if R0 != 0
  };

  {
    // Create a dummy cartridge file with the test program
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute Countdown Loop")
  {
    cpu.reset();

    // MOV R0, #3
    cpu.step();
    REQUIRE(cpu.getRegister(0) == 3); // R0 should be 3 after initialization

    // Iteration 1
    cpu.step(); // SUB R0, R0, #1                   -> R0 is 2
    cpu.step(); // CMP R0, #0                       -> Z flag should be 0 (not zero)
    cpu.step(); // BNE (should branch back to SUB)  -> Pipeline flushed, PC should jump back to SUB instruction
    REQUIRE(cpu.getRegister(0) == 2); // R0 should be 2 after first iteration

    // Iteration 2
    cpu.step(); // SUB R0, R0, #1                   -> R0 is 1
    cpu.step(); // CMP R0, #0                       -> Z flag should be 0
    cpu.step(); // BNE (should branch back to SUB)  -> Pipeline flushed, PC should jump back to SUB instruction
    REQUIRE(cpu.getRegister(0) == 1); // R0 should be 1 after second iteration

    // Iteration 3
    cpu.step(); // SUB R0, R0, #1                           -> R0 is 0
    cpu.step(); // CMP R0, #0                               -> Z flag should be 1
    cpu.step(); // BNE                                      -> Condition fails, PC should move to next instruction
    REQUIRE(cpu.getRegister(0) == 0); // R0 should be 0 after the loop

    // PC should be at the instruction after the BNE (0x08000010 + 8)
    REQUIRE(cpu.getRegister(15) == 0x08000018);
  }

  std::filesystem::remove(testGbaPath);
}
