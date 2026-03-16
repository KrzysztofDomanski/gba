#include <catch2/catch_test_macros.hpp>

#include <thumb_decoder.h>

TEST_CASE("ThumbDecoder Format 3 Parsing", "[thumb][decoder]")
{
  SECTION("Decode MOV R2, #0x55")
  {
    // Hex: 0x2255
    // Binary: 00100 010 01010101
    uint16_t opcode = 0x2255;

    gba::ThumbInstruction inst = gba::ThumbDecoder::decode(opcode);
    REQUIRE(inst.format == 3);
    REQUIRE(inst.opcode == 0b00); // MOV
    REQUIRE(inst.rd == 2);        // R2
    REQUIRE(inst.immediate == 0x55);
  }

  SECTION("Decode ADD R7, #0x12")
  {
    // Hex: 0x3712
    // Binary: 00110 111 00010010
    // Format 3: 001 | Op: 10 (ADD) | Rd: 111 (R7) | Imm: 0x12
    uint16_t opcode = 0x3712;

    gba::ThumbInstruction inst = gba::ThumbDecoder::decode(opcode);
    REQUIRE(inst.format == 3);
    REQUIRE(inst.opcode == 2); // ADD
    REQUIRE(inst.rd == 7);     // R7
    REQUIRE(inst.immediate == 0x12);
  }
}
