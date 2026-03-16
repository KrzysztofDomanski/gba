#include <thumb_alu.h>

using namespace gba;

void ThumbALU::executeFormat3(const ThumbInstruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr)
{
  uint32_t rdVal = registers[inst.rd];
  uint32_t imm = inst.immediate;
  uint32_t result = 0;

  switch (inst.opcode) {
    case 0b00: // MOV
      result = imm;
      registers[inst.rd] = result;
      break;

    case 0b01: // CMP
      result = rdVal - imm;
      break;

    case 0b10: // ADD
      result = rdVal + imm;
      registers[inst.rd] = result;
      break;

    case 0b11: // SUB
      result = rdVal - imm;
      registers[inst.rd] = result;
      break;

    default:
      return; // Invalid opcode, do nothing
  }

  // Update the N and Z flags based on the result
  // THUMB data processing implicitly updates flags
  updateNZFlags(result, cpsr);
}

void ThumbALU::updateNZFlags(uint32_t result, uint32_t& cpsr)
{
  if (result == 0) {
    cpsr |= (1 << 30); // Set Z flag
  } else {
    cpsr &= ~(1 << 30); // Clear Z flag
  }

  if (result & 0x80000000) {
    cpsr |= (1 << 31); // Set N flag
  } else {
    cpsr &= ~(1 << 31); // Clear N flag
  }
}
