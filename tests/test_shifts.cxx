#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU ALU Shifted Register Operands", "[cpu][alu][shifter]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "dummy_cartridge.gba";

  // Assembly Program:
  // 1. MOV R0, #5         -> Set R0 to 5
  //    Hex: 0xE3A00005
  // 2. MOV R1, #10        -> Set R1 to 10
  //    Hex: 0xE3A0100A
  // 3. ADD R2, R0, R1, LSL #3 (R2 = R0 + (R1 << 3)) -> R2 should be 5 + (10 << 3) = 5 + 80 = 85
  //    Hex: 0xE0802181

  std::vector<uint32_t> testProgram = {
      0xE3A00005, // MOV R0, #5
      0xE3A0100A, // MOV R1, #10
      0xE0802181  // ADD R2, R0, R1, LSL #3
  };

  {
    // Create a dummy cartridge file with the test program
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(testProgram.data()), testProgram.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute ADD with LSL")
  {
    cpu.reset();

    // MOV R0, #5
    cpu.step();
    REQUIRE(cpu.getRegister(0) == 5); // R0 should be 5

    // MOV R1, #10
    cpu.step();
    REQUIRE(cpu.getRegister(1) == 10); // R1 should be 10

    // ADD R2, R0, R1, LSL #3
    cpu.step();
    REQUIRE(cpu.getRegister(2) == 85); // R2 should be 85 (5 + (10 << 3))
  }

  std::filesystem::remove(testGbaPath);
}
