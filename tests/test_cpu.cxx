#include <catch2/catch_test_macros.hpp>
#include <cpu.h>

TEST_CASE("CPU Initialization and State", "[CPU]")
{
  gba::Bus bus;
  gba::CPU cpu(bus);

  cpu.reset();

  SECTION("Reset sets PC to ROM entry point")
  {
    cpu.reset();

    // PC should point to ROM entry point
    REQUIRE(cpu.getRegisters(15) == 0x08000000);
    REQUIRE(cpu.getCPSR() == 0x0000001F);
  }

  SECTION("Step increments Program Counter")
  {
    cpu.reset();
    cpu.step();

    // After one step, PC should have advanced by 4 bytes (size of an ARM instruction)
    REQUIRE(cpu.getRegisters(15) == 0x08000004);
  }
}