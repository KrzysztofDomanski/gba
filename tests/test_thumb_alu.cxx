#include <catch2/catch_test_macros.hpp>

#include <thumb_alu.h>

TEST_CASE("", "")
{
  std::array<uint32_t, 16> registers{};
  uint32_t cpsr = 0;

  SECTION("Execute MOV R5, #0x42")
  {
    gba::ThumbInstruction inst{};
    inst.format = 3;
    inst.opcode = 0; // MOV
    inst.rd = 5;     // R5
    inst.immediate = 0x42;

    gba::ThumbALU::executeFormat3(inst, registers, cpsr);

    REQUIRE(registers[5] == 0x42);
    REQUIRE((cpsr & (1 << 30)) == 0); // Z flag should be clear
    REQUIRE((cpsr & (1 << 31)) == 0); // N flag should be clear
  }

  SECTION("Execute SUB R2, #10 to trigger Zero Flag")
  {
    registers[2] = 10;

    gba::ThumbInstruction inst{};
    inst.format = 3;
    inst.opcode = 3; // SUB
    inst.rd = 2;     // R2
    inst.immediate = 10;

    gba::ThumbALU::executeFormat3(inst, registers, cpsr);

    REQUIRE(registers[2] == 0);       // Result should be zero
    REQUIRE((cpsr & (1 << 30)) != 0); // Z flag should be set
    REQUIRE((cpsr & (1 << 31)) == 0); // N flag should be clear
  }
}
