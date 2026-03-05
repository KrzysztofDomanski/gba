#include <lsu.h>

#include <bit>

using namespace gba;

void LSU::executeSingleDataTransfer(const Instruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr,
                                    Bus& bus)
{
  // Base address is in Rn
  uint32_t baseAddress = registers[inst.rn];

  // Calculate the offset. Format 010 means the offset is a 12-bit immediate value.
  // If it was 011 it would be a shifted register - TODO
  uint32_t offset = inst.memoryOffset;

  uint32_t targetAddress = baseAddress;

  // P Bit determines PRE or POST indexing
  // Pre-indexing (P = 1) means add/sub the offset BEFORE addressing the memory
  if (inst.pBit) {
    // U = 1 (UP). U = 2 (DOWN).
    if (inst.uBit) {
      targetAddress += offset;
    } else {
      targetAddress -= offset;
    }
  }

  // The L Bit determines Load (1) or Store (0)
  if (inst.lBit) {
    registers[inst.rd] = bus.read32(targetAddress);
  } else {
    bus.write32(targetAddress, registers[inst.rd]);
  }

  // Post-indexing (P = 0) means add/sub the offset AFTER addressing the memory
  // OR Write-Back (W = 1) means we update the base register with the new address
  if (!inst.pBit || inst.wBit) {
    if (!inst.pBit) {
      // Post indexing always uses the offset applied to the base address
      if (inst.uBit) {
        targetAddress = baseAddress + offset;
      } else {
        targetAddress = baseAddress - offset;
      }
    }
    // Write the calculated address back to Rn
    registers[inst.rn] = targetAddress;
  }
}

void LSU::executeBlockDataTransfer(const Instruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr,
                                   Bus& bus)
{
  uint32_t baseAddress = registers[inst.rn];

  // Count how many bits are set to 1 in the register list to
  // determine how many registers we will load/store
  int numRegisters = std::popcount(inst.registerList);
  if (numRegisters == 0) {
    // If the register list is empty, the behavior is UNPREDICTABLE.
    // For simplicity, we'll just return without doing anything. TODO handle better
    return;
  }

  uint32_t startAddress = baseAddress;

  // The total size of the transfer block is 4 * numRegisters (each register is 4 bytes)
  uint32_t blockSize = 4 * numRegisters;
  // Calculate the lowest memory address based on the addressing mode (U and P bits)
  if (inst.uBit) {
    // Up
    if (inst.pBit) {
      // (IB) Increment before
      startAddress += 4;
    }
    // (IA) Increment after - startAddress is baseAddress, so no change needed
  } else {
    // Down
    if (inst.pBit) {
      // (DB) Decrement before - Standard PUSH
      startAddress -= blockSize;
    } else {
      // (DA) Decrement after - Standard POP
      startAddress -= 4 * (numRegisters - 1);
    }
  }

  uint32_t currentAddress = startAddress;

  // The ARM CPU always transfers registers in order from R0 to R15,
  // regardless of the order of bits in the register list.
  for (int regIndex = 0; regIndex < 16; regIndex++) {
    // If bit regIndex in the register list is set, we need to transfer that register
    if ((inst.registerList >> regIndex) & 1) {
      if (inst.lBit) {
        // LDM (Load Multiple)
        registers[regIndex] = bus.read32(currentAddress);
      } else {
        // STM (Store Multiple)
        bus.write32(currentAddress, registers[regIndex]);
      }
      // Move to the next word in memory
      currentAddress += 4;
    }
  }

  // Write-back (W bit) means we update the base register with the new address after the transfer
  if (inst.wBit) {
    if (inst.uBit) {
      registers[inst.rn] = baseAddress + blockSize;
    } else {
      registers[inst.rn] = baseAddress - blockSize;
    }
  }
}

void LSU::executeHalfwordTransfer(const Instruction& inst, std::array<uint32_t, 16>& registers, Bus& bus)
{
  uint32_t baseAddress = registers[inst.rn];

  // Determine offset (Immediate vs Register)
  uint32_t offset = 0;
  if (inst.iBit) {
    offset = inst.memoryOffset;
  } else {
    offset = registers[inst.memoryOffset & 0xF];
  }

  uint32_t targetAddress = baseAddress;

  // P Bit determines PRE or POST indexing
  if (inst.pBit) {
    // U = 1 (UP). U = 2 (DOWN).
    if (inst.uBit) {
      targetAddress += offset;
    } else {
      targetAddress -= offset;
    }
  }

  // TODO Only supporting unsigned halfwords (hwOpcode 0b01) for now. Need to implement signed halfword transfers (LDRSH
  // and STRSH) as well.
  if (inst.hwOpcode == 0b01) {
    if (inst.lBit) {
      // LDRH: Load Register Halfword (Zero-extended to 32 bits)
      registers[inst.rd] = bus.read16(targetAddress);
    } else {
      // STRH: Store Register Halfword (Lower 16 bits of the register)
      bus.write16(targetAddress, static_cast<uint16_t>(registers[inst.rd] & 0xFFFF));
    }
  }

  // Post-indexing or Write-Back (W = 1) means we update the base register with the new address
  if (!inst.pBit || inst.wBit) {
    if (!inst.pBit) {
      // Post indexing always uses the offset applied to the base address
      if (inst.uBit) {
        targetAddress = baseAddress + offset;
      } else {
        targetAddress = baseAddress - offset;
      }
    }
    // Write the calculated address back to Rn
    registers[inst.rn] = targetAddress;
  }
}
