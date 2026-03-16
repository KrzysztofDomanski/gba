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
