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

  // Skip the BIOS and point the Program Counter directly at the ROM entry point
  registers[15] = 0x08000000;

  // Set the Stack Pointers to their standard post-BIOS memory locations
  // TODO Implement banked registers for different CPU modes
  registers[13] = 0x03007F00; // Standard IWRAM stack location
}

void CPU::step()
{
  // Fetch
  uint32_t currentPc = registers[15];

  // TODO Actually read the instruction from the bus
  // uint32_t instruction = bus.read32(currentPc);

  // Increment PC
  // Point to the next instruction (assuming 32-bit instruction size)
  registers[15] += 4;

  // TODO Decode & Execute
}

uint32_t CPU::getRegisters(size_t index) const
{
  return index >= 16 ? 0 : registers[index];
}
