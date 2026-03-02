#include <bus.h>
#include <catch2/catch_test_macros.hpp>
#include <cpu.h>
#include <lsu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU Block Data Transfer (Stack Push/Pop)", "[cpu][lsu][stack]")
{
  gba::Bus bus;

  std::filesystem::path tempRomPath = "test_stack.gba";

  // Assembly Program:
  //
  // 1. MOV R0, #10           Hex: 0xE3A0000A
  // 2. MOV R1, #20           Hex: 0xE3A01014
  // 3. PUSH {R0, R1}         (Alias for STMDB R13!, {R0, R1})
  //    Format: Cond(E) | Fmt(100) | P(1) U(0) S(0) W(1) L(0) | Rn(1101) | RegList(0000000000000011)
  //    Hex: 0xE92D0003
  // 4. POP {R2, R3}          (Alias for LDMIA R13!, {R2, R3})
  //    Format: Cond(E) | Fmt(100) | P(0) U(1) S(0) W(1) L(1) | Rn(1101) | RegList(0000000000001100)
  //    Hex: 0xE8BD000C

  std::vector<uint32_t> instructions = {
      0xE3A0000A, // MOV R0, #10
      0xE3A01014, // MOV R1, #20
      0xE92D0003, // PUSH {R0, R1}
      0xE8BD000C  // POP {R2, R3}
  };

  {
    std::ofstream file(tempRomPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(tempRomPath) == true);

  gba::CPU cpu(bus);

  SECTION("Execute Block Data Transfer")
  {
    cpu.reset();
    // R13 initializes to 0x03007F00 in reset()
    REQUIRE(cpu.getRegister(13) == 0x03007F00);

    // Execute MOV R0, #10
    cpu.step();

    // Execute MOV R1, #20
    cpu.step();

    // Execute PUSH {R0, R1}
    cpu.step();
    // Stack grows downward. Two registers pushed = 8 bytes.
    // R13 should now be write-backed to 0x03007EF8
    REQUIRE(cpu.getRegister(13) == 0x03007F00 - 8); // Stack pointer should decrement by 8

    // R0 (10) should be at the lower address, R1 (20) at the higher address
    REQUIRE(bus.read32(0x03007EF8) == 10);
    REQUIRE(bus.read32(0x03007EFC) == 20);

    // Execute POP {R2, R3}
    cpu.step();
    REQUIRE(cpu.getRegister(2) == 10);
    REQUIRE(cpu.getRegister(3) == 20);

    REQUIRE(cpu.getRegister(13) == 0x03007F00); // Stack pointer should be restored
  }

  std::filesystem::remove(tempRomPath);
}
