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

  // Detect Format 5 where the top 6 bits are 010001 (0x11)
  if ((rawOpcode >> 10) == 0x11) {
    inst.format = 5;
    inst.opcode = (rawOpcode >> 8) & 0x3; // 0=ADD, 1=CMP, 2=MOV, 3=BX
    inst.h1 = (rawOpcode >> 7) & 0x1;     // Bit 7 is H1
    inst.h2 = (rawOpcode >> 6) & 0x1;     // Bit 6 is H2
    inst.rs = (rawOpcode >> 3) & 0x7;     // Bits 3-5 for source register
    inst.rd = rawOpcode & 0x7;            // Bits 0-2 for destination register
    return inst;
  }

  // Detect Format 9 where the top 3 bits are 011
  if ((rawOpcode >> 13) == 0b011) {
    inst.format = 9;
    inst.bBit = (rawOpcode >> 12) & 0x1;      // Bit 12 is the B bit
    inst.lBit = (rawOpcode >> 11) & 0x1;      // Bit 11 is the L bit
    inst.immediate = (rawOpcode >> 6) & 0x1F; // 5-bit offset
    inst.rb = (rawOpcode >> 3) & 0x7;         // Bits 3-5 for base register
    inst.rd = rawOpcode & 0x7;                // Bits 0-2 for destination register
    return inst;
  }

  // Detect Format 14: Push/Pop Registers
  // The top 4 bits are 1011 (0xB), and bits 9-10 are 10
  if ((rawOpcode >> 12) == 0xB && ((rawOpcode >> 9) & 0x3) == 0b10) {
    inst.format = 14;
    inst.lBit = (rawOpcode >> 11) & 0x1;  // Bit 11 is the L bit (1 for POP, 0 for PUSH)
    inst.rBit = (rawOpcode >> 8) & 0x1;   // Bit 8 for R bit
    inst.registerList = rawOpcode & 0xFF; // Bits 0-7 for register list
    return inst;
  }

  // Detect Format 19: Branch with Link (BL)
  if ((rawOpcode >> 12) == 0xF) {
    inst.format = 19;
    inst.isBlSuffix = (rawOpcode >> 11) & 0x1; // Bit 11 distinguishes prefix (0) from suffix (1)
    inst.linkOffset = rawOpcode & 0x7FF;       // Bits 0-10 for the link offset
    return inst;
  }

  // Detect format 16: Conditional branch
  // Top 4 bits are 1101
  if ((rawOpcode >> 12) == 0xD) {
    inst.format = 16;
    inst.cond = (rawOpcode >> 8) & 0xF;

    int8_t offset = rawOpcode & 0xFF; // Bits 0-7 for signed offset
    inst.branchOffset = static_cast<int32_t>(offset);
    return inst;
  }

  // Catch all for unimplemented formats
  inst.format = 99;
  return inst;
}
