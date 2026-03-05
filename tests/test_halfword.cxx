#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU Halfword Data Transfer (STRH)", "[cpu][lsu][halfword]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "test_halfword.gba";

  // Prefill memory with a known value to verify the halfword store is working correctly
  bus.write32(0x03000000, 0xFFFFFFFF);

  // Assembly Program:
  // 1. MOV R0, #0x03000000
  //    Hex: 0xE3A00403
  // 2. MOV R1, #0xAA
  //    Hex: 0xE3A010AA
  // 3. STRH R1, [R0]
  //    Format: Cond(E) | 000 | P(1) U(1) I(1) W(0) L(0) | Rn(0000) | Rd(0001) | OffHi(0000) | 1 | S(0) H(1) 1 |
  //    OffLo(0000) Hex: 0xE1C010B0

  std::vector<uint32_t> instructions = {
      0xE3A00403, // MOV R0, #0x03000000
      0xE3A010AA, // MOV R1, #0xAA
      0xE1C010B0  // STRH R1, [R0]
  };

  {
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute STRH and Verify Memory")
  {
    cpu.reset();

    // Execute MOV R0, #0x03000000
    cpu.step();
    // Execute MOV R1, #0xAA
    cpu.step();

    REQUIRE(bus.read32(0x03000000) == 0xFFFFFFFF); // Memory should be unchanged before the STRH

    // Execute STRH R1, [R0]
    cpu.step();

    // After storing the halfword, the memory at 0x03000000 should be 0xFFFF00AA (lower 16 bits updated)
    REQUIRE(bus.read32(0x03000000) == 0xFFFF00AA);

    REQUIRE(bus.read16(0x03000000) == 0x00AA); // Reading the halfword directly should give us 0x00AA
  }

  std::filesystem::remove(testGbaPath);
}