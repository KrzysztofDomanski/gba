#include <bus.h>

using namespace gba;

Bus::Bus() = default;

uint8_t Bus::read8(uint32_t address) const {
  // GBA memory regions are determined by the top 8 bits (address >> 24)
  uint8_t region = address >> 24 & 0xFF;

  switch (region) {
  case 0x02:                         // EWRAM
    return ewram[address & 0x3FFFF]; // Mask to 256 KB
  case 0x03:                         // IWRAM
    return iwram[address & 0x7FFF];  // Mask to 32 KB
  default:
    // For now, return 0 for unmapped regions
    return 0;
  }
}

void Bus::write8(uint32_t address, uint8_t value) {
  uint8_t region = address >> 24 & 0xFF;

  switch (region) {
  case 0x02:                          // EWRAM
    ewram[address & 0x3FFFF] = value; // Mask to 256 KB
    break;
  case 0x03:                         // IWRAM
    iwram[address & 0x7FFF] = value; // Mask to 32 KB
    break;
  default:
    // Ignore writes to unmapped regions for now
    break;
  }
}

// 16-bit and 32-bit accesses are little-endiant and must be aligned
uint16_t Bus::read16(uint32_t address) const {
  // Force 16 bit alignment by clearing the least significant bit
  address &= ~1;
  return read8(address) | (read8(address + 1) << 8);
}

void Bus::write16(uint32_t address, uint16_t value) {
  address &= ~1; // Force 16 bit alignment
  write8(address, value & 0xFF);
  write8(address + 1, (value >> 8) & 0xFF);
}

uint32_t Bus::read32(uint32_t address) const {
  // Force 32 bit alignment by clearing the least significant 2 bits
  address &= ~3;
  return read8(address) | (read8(address + 1) << 8) |
         (read8(address + 2) << 16) | (read8(address + 3) << 24);
}

void Bus::write32(uint32_t address, uint32_t value) {
  address &= ~3; // Force 32 bit alignment
  write8(address, value & 0xFF);
  write8(address + 1, (value >> 8) & 0xFF);
  write8(address + 2, (value >> 16) & 0xFF);
  write8(address + 3, (value >> 24) & 0xFF);
}
