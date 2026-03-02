#include <bus.h>
#include <catch2/catch_test_macros.hpp>
#include <cpu.h>
#include <lsu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU Load/Store (Single Data Transfer)", "[cpu][lsu]")
{
  gba::Bus bus;

  std::filesystem::path tempRomPath = "test_lsu.gba";

  // Assembly Program:
  // 1. MOV R0, #0x03000000 <- Set R0 as base address pointing to IWRAM
  //    Hex: 0xE3A00403     <- Using Barrel Shifter: 0x03 rotate right by 8 = 0x03000000
  // 2. MOV R1, #42         <- Load the value 42 to store in R1
  //    Hex: 0xE3A0102A
  // 3. STR R1, [R0]        <- Store the value in R1 to the address pointed by R0. Pre-indexed, Offset = 0(IWRAM)
  //    Hex: 0xE5801000
  // 4. LDR R2, [R0]        <- Load the value from the address pointed by R0 into R2
  //    Hex: 0xE5902000

  std::vector<uint32_t> instructions = {
      0xE3A00403, // MOV R0, #0x03000000
      0xE3A0102A, // MOV R1, #42
      0xE5801000, // STR R1, [R0]
      0xE5902000  // LDR R2, [R0]
  };

  // Write the raw binary instructions to disk
  {
    std::ofstream file(tempRomPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(tempRomPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute STR and LDR")
  {
    cpu.reset();

    // Execute MOV R0, #0x03000000
    cpu.step();
    REQUIRE(cpu.getRegister(0) == 0x03000000);

    // Execute MOV R1, #42
    cpu.step();
    REQUIRE(cpu.getRegister(1) == 42);

    // Execute STR R1, [R0]
    cpu.step();
    // Verify that the bus received the write request
    REQUIRE(bus.read32(0x03000000) == 42);

    // Execute LDR R2, [R0]
    cpu.step();
    REQUIRE(cpu.getRegister(2) == 42);
  }

  std::filesystem::remove(tempRomPath);
}