#include <bus.h>
#include <catch2/catch_test_macros.hpp>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU Conditional Execution", "[cpu][conditions]")
{
  gba::Bus bus;
  std::filesystem::path testGbaPath = "dummy_cartridge.gba";

  // Assembly Program:
  // 1. MOVS R0, #0         -> Set R0 to 0, which sets the Zero flag (Z) in CPSR
  //    Hex: 0xE3B00000     -> Cond: E = AL
  // 2. ADDNE R1, R1, #5    -> Add 5 to R1 IF Not Equal / Zero clear (Should fail)
  //    Hex: 0x12811005     -> Cond: 1 = NE
  // 3. ADDEQ R2, R2, #10   -> Add 10 to R2 IF Equal / Zero set (Should succeed)
  //    Hex: 0x0282200A     -> Cond: 0 = EQ

  std::vector<uint32_t> testProgram = {
      0xE3B00000, // MOVS R0, #0
      0x12811005, // ADDNE R1, R1, #5
      0x0282200A  // ADDEQ R2, R2, #10
  };

  {
    // Create a dummy cartridge file with the test program
    std::ofstream file(testGbaPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(testProgram.data()), testProgram.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(testGbaPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute EQ and NE Conditions")
  {
    cpu.reset();

    // MOVS R0, #0
    cpu.step();
    REQUIRE(cpu.getRegister(0) == 0);     // R0 should be 0
    REQUIRE((cpu.getCPSR() >> 30) & 0x1); // Z flag should be set

    // Attempt ADDNE R1, R1, #5 (should not execute because Z is set)
    cpu.step();
    REQUIRE(cpu.getRegister(1) == 0); // R1 should remain 0

    // Attempt ADDEQ R2, R2, #10 (should execute because Z is set)
    cpu.step();
    REQUIRE(cpu.getRegister(2) == 10); // R2 should be 10
  }

  std::filesystem::remove(testGbaPath);
}
