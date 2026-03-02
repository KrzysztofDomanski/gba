#include <lsu.h>

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
