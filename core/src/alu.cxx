#include <alu.h>

#include <bit>

using namespace gba;

void ALU::executeDataProcessing(const Instruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr)
{
  uint32_t result = 0;

  // The Barrel Shifter processes the operand BEFORE the ALU gets it
  uint32_t op2Value = getShiftedOperand2(inst);
  uint32_t op1Value = registers[inst.rn];

  switch (inst.aluOpcode) {
  case 0x4: // ADD
    result = op1Value + op2Value;
    break;
  case 0xD: // MOV
    result = op2Value;
    break;
  default:
    return;
  }

  registers[inst.rd] = result;

  if (inst.sBit) {
    updateNZFlags(result, cpsr);
  }
}

void ALU::updateNZFlags(uint32_t result, uint32_t& cpsr)
{
  // Zero Flag (Bit 30): Set if the result is exactly zero
  if (result == 0) {
    // Set Z bit
    cpsr |= (1 << 30);
  } else {
    // Clear Z bit
    cpsr &= ~(1 << 30);
  }

  // Negative Flag (Bit 31): Set if the result is negative (i.e., if the most significant bit is 1)
  if (result & 0x80000000) {
    // Set N bit
    cpsr |= (1 << 31);
  } else {
    // Clear N bit
    cpsr &= ~(1 << 31);
  }
}

uint32_t ALU::getShiftedOperand2(const Instruction& inst)
{
  if (!inst.iBit)
    return 0;

  // Bottom 8 bits
  uint32_t imm8 = inst.operand2 & 0xFF;

  // Top 4 bits
  uint32_t rotateAmount = (inst.operand2 >> 8) & 0xF;

  // The ARM spec says the rotate amount is doubled
  uint32_t shift = rotateAmount * 2;

  if (shift == 0)
    return imm8;

  // 32-bit right rotation
  return std::rotr(imm8, shift);
}