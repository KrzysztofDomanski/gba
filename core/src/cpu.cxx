#include <alu.h>
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
  decodedInstruction = Decoder::decodeARM(fetchedOpcode);
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
    ALU::executeDataProcessing(decodedInstruction, registers, currentProgramStatusRegister);
    break;
  default:
    break;
  }
}

uint32_t CPU::getRegister(size_t index) const
{
  return index >= 16 ? 0 : registers[index];
}
