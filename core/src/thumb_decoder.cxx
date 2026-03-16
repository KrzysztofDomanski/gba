#include <thumb_decoder.h>

using namespace gba;

ThumbInstruction ThumbDecoder::decode(uint16_t rawOpcode)
{
  ThumbInstruction inst{};
  inst.rawOpcode = rawOpcode;

  // Detect format 3 for move, compare, add, and subtract instructions with register operands
  if ((rawOpcode >> 13) == 0b001) {
    inst.format = 3;
    inst.opcode = (rawOpcode >> 11) & 0x3; // Bits 11-12 identify the specific math operation
    inst.rd = (rawOpcode >> 8) & 0x7;      // Bits 8-10 for destination register
    inst.immediate = rawOpcode & 0xFF;     // Bits 0-7 for immediate value
    return inst;
  }

  // Catch all for unimplemented formats
  inst.format = 99;
  return inst;
}
