#include <alu.h>

#include <bit>

using namespace gba;

void ALU::executeDataProcessing(const Instruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr)
{
  uint32_t result = 0;

  // The Barrel Shifter processes the operand BEFORE the ALU gets it
  uint32_t op2Value = getShiftedOperand2(inst, registers);
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

uint32_t ALU::getShiftedOperand2(const Instruction& inst, const std::array<uint32_t, 16>& registers)
{
  if (inst.iBit) {
    // Immediate rotation

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

  // Register shift
  uint32_t value = registers[inst.rm];

  // Get the shift amount either from register or the 5-bit immediate
  uint32_t shift = inst.shiftByRegister ? registers[inst.rs] & 0xFF : inst.shiftAmount;

  // This is a hardware edge case, shifting by 0 does nothing
  // TODO There's a Carry flag edge case which needs doing
  if (shift == 0)
    return value;

  switch (inst.shiftType) {
  case 0x0: // LSL (logical shift left)
    if (shift >= 32)
      return 0;
    return value << shift;

  case 0x1: // LSR (logical shift right)
    if (shift >= 32)
      return 0;
    return value >> shift;

  case 0x2: // ASR (arithmetic shift right)
    if (shift >= 32) {
      // If signed bit is 1, fill with 1s, otherwise fill with 0s
      return (value & 0x80000000) ? 0xFFFFFFFF : 0;
    }
    return static_cast<int32_t>(value) >> shift;

  case 0x3: // ROR (rotate right)
    // Rotate amounts are effectively mod 32
    shift = shift % 32;
    if (shift == 0)
      return value;
    return std::rotr(value, shift);

  default:
    return value;
  }
}