#include <catch2/catch_test_macros.hpp>
#include <cpu.h>
#include <filesystem>
#include <fstream>

TEST_CASE("CPU Pipeline and PC Offset", "[cpu]")
{
  gba::Bus bus;

  // Inject a dummy Data Processing opcode (e.g., an ADD instruction) at the entry point
  // 0xE0810002 is 'ADD R0, R1, R2' in ARM
  bus.write32(0x08000000, 0xE0810002);

  gba::CPU cpu(bus);

  SECTION("Pipeline fills correctly on reset")
  {
    cpu.reset();

    // After reset, the pipeline is full.
    // The PC (R15) should be pointing 8 bytes ahead of the entry point.
    REQUIRE(cpu.getRegister(15) == 0x08000008);
    REQUIRE(cpu.getCPSR() == 0x0000001F);
  }

  SECTION("Stepping maintains pipeline offset")
  {
    cpu.reset();
    // Executes instruction at 0x08000000, PC moves to 0x0800000C
    cpu.step();

    REQUIRE(cpu.getRegister(15) == 0x0800000C);
  }
}

TEST_CASE("CPU ALU Data Processing", "[cpu][alu]")
{
  gba::Bus bus;

  std::filesystem::path tempRomPath = "test_alu.gba";
  std::vector<uint32_t> instructions = {
      0xE3B00005, // MOVS R0, #5
      0xE290100A, // ADDS R1, R0, #10
      0xE3B02000  // MOVS R2, #0
  };

  // Write the raw binary instructions to disk
  {
    std::ofstream file(tempRomPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(tempRomPath) == true);

  gba::CPU cpu(bus);

  SECTION("Execute MOVS and ADDS")
  {
    cpu.reset();

    // Execute the MOVS instruction
    cpu.step();
    REQUIRE(cpu.getRegister(0) == 5);
    REQUIRE((cpu.getCPSR() & (1 << 30)) == 0); // Z flag should be clear

    // Execute the ADDS instruction
    cpu.step();
    REQUIRE(cpu.getRegister(1) == 15);         // R0(5) + 10 = 15
    REQUIRE((cpu.getCPSR() & (1 << 30)) == 0); // Z flag should be clear

    cpu.step();
    REQUIRE(cpu.getRegister(2) == 0);
    REQUIRE((cpu.getCPSR() & (1 << 30)) != 0); // Z flag should be set
  }

  std::filesystem::remove(tempRomPath);
}

TEST_CASE("CPU Barrel Shifter Immediate", "[cpu][alu][barrel_shifter]")
{
  gba::Bus bus;

  std::filesystem::path tempRomPath = "test_barrel_shifter.gba";
  // MOV R0, #0xFF000000
  std::vector<uint32_t> instructions = {0xE3A004FF};

  // Write the raw binary instructions to disk
  {
    std::ofstream file(tempRomPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(tempRomPath) == true);

  gba::CPU cpu(bus);

  SECTION("Execute Rotated Immediate MOV")
  {
    cpu.reset();
    cpu.step();

    // Verify the 8-bit value was safely rotated across the 32-bit boundary
    REQUIRE(cpu.getRegister(0) == 0xFF000000);
  }

  std::filesystem::remove(tempRomPath);
}
