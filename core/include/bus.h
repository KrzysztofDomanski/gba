#pragma once

#include <array>
#include <cstdint>

namespace gba {

/// The GBA's CPU (ARM7TDMI) never talks directly to ROM, RAM, or the
/// screen. Instead, it asks the "Bus" to fetch or store data at specific
/// 32-bit memory addresses.
///
/// The GBA uses a unified address space. Depending on the memory address
/// the CPU asks for, the Bus routes the request to Work RAM, Video RAM,
/// I/O registers, or the cartridge ROM.
///
/// The GBA is a little-endian system, so multi-byte values are stored with
/// the least significant byte at the lowest memory address.
class Bus {
public:
  Bus();
  ~Bus() = default;

  // The GBA uses a 32-bit address bus
  [[nodiscard]] uint8_t read8(uint32_t address) const;
  [[nodiscard]] uint16_t read16(uint32_t address) const;
  [[nodiscard]] uint32_t read32(uint32_t address) const;

  void write8(uint32_t address, uint8_t value);
  void write16(uint32_t address, uint16_t value);
  void write32(uint32_t address, uint32_t value);

private:
  // External Work Ram (EWRAM) - 256 KB (0x02000000 - 0x0203FFFF)
  std::array<uint8_t, 256 * 1024> ewram{};

  // Internal Work Ram (IWRAM) - 32 KB (0x03000000 - 0x03007FFF)
  std::array<uint8_t, 32 * 1024> iwram{};
};
} // namespace gba
