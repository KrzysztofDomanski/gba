#include <cpu.h>

using namespace gba;

CPU::CPU(Bus& bus_) : bus(bus_)
{
}

void CPU::reset()
{
  // Reset all registers to 0
  registers.fill(0);

  // System Mode, ARM state, IRQ/FIQ (Interrupt Request, Fast Interrupt Request) disabled
  currentProgramStatusRegister = 0x0000001F;

  // The PC (R15) points to the current instruction being FETCHED.
  // Because of the pipeline, when execution starts at 0x08000000,
  // the PC will already be at 0x08000008.
  registers[15] = 0x08000000;

  fetch();
  decode();
  fetch();
}

void CPU::step()
{
  execute();
  decode();
  fetch();
}

void CPU::fetch()
{
  fetchedOpcode = bus.read32(registers[15]);
  registers[15] += 4;
}

void CPU::decode()
{
  uint32_t op = fetchedOpcode;
  decodedInstruction.rawOpcode = op;

  decodedInstruction.iBit = (op >> 25) & 0x1;
  decodedInstruction.aluOpcode = (op >> 21) & 0xF;
  decodedInstruction.sBit = (op >> 20) & 0x1;
  decodedInstruction.rn = (op >> 16) & 0xF;
  decodedInstruction.rd = (op >> 12) & 0xF;
  decodedInstruction.operand2 = op & 0xFFF;
}

void CPU::execute()
{
  uint32_t opcode = decodedInstruction.rawOpcode;
  uint8_t format = (opcode >> 25) & 0x7;

  switch (format) {
  case 0b000:
    [[fallthrough]];
  case 0b001:
    // Bits 25-27 being 000 or 001 usually means a Data Processing instruction
    // (e.g., ADD, SUB, AND, ORR)
    executeDataProcessing(opcode);
    break;
  default:
    break;
  }
}

void CPU::executeDataProcessing(uint32_t opcode)
{
  const auto& inst = decodedInstruction;
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
    updateNZFlags(result);
  }
}

void CPU::updateNZFlags(uint32_t result)
{
  // Zero Flag (Bit 30): Set if the result is exactly zero
  if (result == 0) {
    // Set Z bit
    currentProgramStatusRegister |= (1 << 30);
  } else {
    // Clear Z bit
    currentProgramStatusRegister &= ~(1 << 30);
  }

  // Negative Flag (Bit 31): Set if the result is negative (i.e., if the most significant bit is 1)
  if (result & 0x80000000) {
    // Set N bit
    currentProgramStatusRegister |= (1 << 31);
  } else {
    // Clear N bit
    currentProgramStatusRegister &= ~(1 << 31);
  }
}

uint32_t CPU::getShiftedOperand2(const Instruction& inst) const
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

uint32_t CPU::getRegister(size_t index) const
{
  return index >= 16 ? 0 : registers[index];
}
