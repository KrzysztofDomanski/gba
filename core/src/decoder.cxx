#include <decoder.h>

using namespace gba;

Instruction Decoder::decodeARM(uint32_t opcode)
{
  Instruction inst;
  inst.rawOpcode = opcode;

  // Condition code is always in bits 28-31
  inst.conditionCode = (opcode >> 28) & 0xF;

  // Bits 25-27 determine the instruction format
  inst.format = (opcode >> 25) & 0x7;

  // Detect halfwrod/signed data transfer instructions by checking for the specific bit pattern in bits 4-7
  inst.isHalfwordTransfer = (inst.format == 0b000) && ((opcode >> 4) & 0x1) == 1 && ((opcode >> 7) & 0x1) == 1;

  if (inst.isHalfwordTransfer) {
    inst.pBit = (opcode >> 24) & 0x1;
    inst.uBit = (opcode >> 23) & 0x1;
    inst.iBit = (opcode >> 22) & 0x1;
    inst.wBit = (opcode >> 21) & 0x1;
    inst.lBit = (opcode >> 20) & 0x1;
    inst.rn = (opcode >> 16) & 0xF;
    inst.rd = (opcode >> 12) & 0xF;
    inst.hwOpcode = (opcode >> 5) & 0x3; // Bits 5-6 specify the halfword/signed transfer type

    // The 8-bit immediate offset is split across bits 0-3 and 8-11, so we need to combine them
    uint8_t offsetHi = (opcode >> 8) & 0xF;         // Bits 8-11
    uint8_t offsetLo = opcode & 0xF;                // Bits 0-3
    inst.memoryOffset = (offsetHi << 4) | offsetLo; // Combine to get the full 8-bit offset
  } else {

    // Data Processing fields
    inst.iBit = (opcode >> 25) & 0x1;
    inst.aluOpcode = (opcode >> 21) & 0xF;
    inst.sBit = (opcode >> 20) & 0x1;
    inst.rn = (opcode >> 16) & 0xF;
    inst.rd = (opcode >> 12) & 0xF;

    if (inst.iBit) {
      inst.operand2 = opcode & 0xFFF;
    } else {
      inst.rm = opcode & 0xF;
      inst.shiftByRegister = (opcode >> 4) & 0x1;
      inst.shiftType = (opcode >> 5) & 0x3;

      if (inst.shiftByRegister) {
        inst.rs = (opcode >> 8) & 0xF;
      } else {
        inst.shiftAmount = (opcode >> 7) & 0x1F;
      }
    }

    // Branch fields
    inst.linkBit = (opcode >> 24) & 0x1;
    uint32_t offset = opcode & 0x00FFFFFF; // 24-bit signed offset
    if (offset & 0x00800000) {             // If the sign bit (bit 23) is set
      offset |= 0xFF000000;                // Sign-extend to 32 bits
    }
    inst.branchOffset = static_cast<int32_t>(offset);

    // Load/Store fields
    inst.pBit = (opcode >> 24) & 0x1;
    inst.uBit = (opcode >> 23) & 0x1;
    inst.bBit = (opcode >> 22) & 0x1;
    inst.wBit = (opcode >> 21) & 0x1;
    inst.lBit = (opcode >> 20) & 0x1;
    inst.memoryOffset = opcode & 0xFFF;

    // Block Data Transfer fields
    inst.registerList = opcode & 0xFFFF;
  }

  return inst;
}