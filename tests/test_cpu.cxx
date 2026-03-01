#include <catch2/catch_test_macros.hpp>
#include <cpu.h>

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