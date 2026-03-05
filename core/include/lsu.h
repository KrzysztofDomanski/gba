#pragma once

#include <bus.h>
#include <decoder.h>

#include <array>
#include <cstdint>

namespace gba
{
  class LSU {
  public:
    // executeSingleDataTransfer() takes a decoded instruction and performs the appropriate Load or Store operation
    // based on the pBit, uBit, bBit, wBit and lBit fields.
    static void executeSingleDataTransfer(const Instruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr,
                                          Bus& bus);

    // executeBlockDataTransfer() takes a decoded instruction and performs the appropriate Load or Store operation for
    // block data transfer instructions (Format 100) based on the pBit, uBit, wBit and lBit fields.
    static void executeBlockDataTransfer(const Instruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr,
                                         Bus& bus);

    // executeHalfwordTransfer() takes a decoded instruction and performs the appropriate Load or Store operation for
    // halfword and signed data transfer instructions based on the pBit, uBit, bBit, wBit, lBit and hwOpcode fields.
    static void executeHalfwordTransfer(const Instruction& inst, std::array<uint32_t, 16>& registers, Bus& bus);
  };
} // namespace gba
