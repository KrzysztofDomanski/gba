#include <decoder.h>

using namespace gba;

Instruction Decoder::decodeARM(uint32_t opcode)
{
  Instruction inst;
  inst.rawOpcode = opcode;

  // Data Processing fields
  inst.iBit = (opcode >> 25) & 0x1;
  inst.aluOpcode = (opcode >> 21) & 0xF;
  inst.sBit = (opcode >> 20) & 0x1;
  inst.rn = (opcode >> 16) & 0xF;
  inst.rd = (opcode >> 12) & 0xF;
  inst.operand2 = opcode & 0xFFF;

  // Branch fields
  inst.linkBit = (opcode >> 24) & 0x1;
  uint32_t offset = opcode & 0x00FFFFFF; // 24-bit signed offset
  if (offset & 0x00800000) {             // If the sign bit (bit 23) is set
    offset |= 0xFF000000;                // Sign-extend to 32 bits
  }
  inst.branchOffset = static_cast<int32_t>(offset);

  return inst;
}