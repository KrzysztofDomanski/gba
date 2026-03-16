#include <thumb_lsu.h>

using namespace gba;

void ThumbLSU::executeFormat9(const ThumbInstruction& inst, std::array<uint32_t, 16>& registers, Bus& bus)
{
  uint32_t baseAddress = registers[inst.rb];
  uint32_t offset = inst.immediate;

  if (inst.bBit) {
    // Byte transfer
    uint32_t address = baseAddress + offset;

    if (inst.lBit) {
      // Load byte LDRB
      registers[inst.rd] = bus.read8(address);
    } else {
      // Store byte STRB
      bus.write8(address, registers[inst.rd] & 0xFF);
    }
  } else {
    // Word transfer
    // Offset is multiplied by 4 for word transfers, the hardware does the same
    uint32_t address = baseAddress + (offset << 2);

    // ARM requires Word accesses to be aligned to 4 bytes
    address &= ~0x3;

    if (inst.lBit) {
      // Load word LDR
      registers[inst.rd] = bus.read32(address);
    } else {
      // Store word STR
      bus.write32(address, registers[inst.rd]);
    }
  }
}

bool ThumbLSU::executeFormat14(const ThumbInstruction& inst, std::array<uint32_t, 16>& registers, Bus& bus)
{
  bool pcModified = false;

  int numRegisters = std::popcount(inst.registerList);

  if (inst.rBit) {
    numRegisters += 1; // Account for PC in register list
  }

  uint32_t startAddress = registers[13]; // SP is R13

  if (inst.lBit) {
    // POP Load Multiple Increment After
    uint32_t currentAddress = startAddress;

    // Pop R0-R7
    for (uint8_t reg = 0; reg < 8; ++reg) {
      if (inst.registerList & (1 << reg)) {
        registers[reg] = bus.read32(currentAddress);
        currentAddress += 4;
      }
    }

    // Pop PC if rBit is set
    if (inst.rBit) {
      uint32_t poppedPc = bus.read32(currentAddress);
      // For now we simply align this, but the GBA actually uses
      // the lowest bit for correct BX-like state switching
      registers[15] = poppedPc & ~0x1;
      currentAddress += 4;
      pcModified = true;
    }

    // Update SP
    registers[13] = currentAddress;

  } else {
    // PUSH Store Multiple Decrement Before
    uint32_t blockSize = numRegisters * 4;
    startAddress -= blockSize; // Move SP down by block size
    uint32_t currentAddress = startAddress;

    // Update SP before storing
    registers[13] = currentAddress;

    // Push R0-R7
    for (uint8_t reg = 0; reg < 8; ++reg) {
      if ((inst.registerList >> reg) & 1) {
        bus.write32(currentAddress, registers[reg]);
        currentAddress += 4;
      }
    }

    // Push PC if rBit is set
    if (inst.rBit) {
      bus.write32(currentAddress, registers[14]);
      currentAddress += 4;
    }

    registers[13] = startAddress; // Update SP to new value
  }

  return pcModified;
}
