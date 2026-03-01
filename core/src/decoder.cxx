#include <decoder.h>

using namespace gba;

Instruction Decoder::decodeARM(uint32_t opcode)
{
  Instruction inst;
  inst.rawOpcode = opcode;
  inst.iBit = (opcode >> 25) & 0x1;
  inst.aluOpcode = (opcode >> 21) & 0xF;
  inst.sBit = (opcode >> 20) & 0x1;
  inst.rn = (opcode >> 16) & 0xF;
  inst.rd = (opcode >> 12) & 0xF;
  inst.operand2 = opcode & 0xFFF;

  return inst;
}