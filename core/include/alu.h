#pragma once

#include <decoder.h>

#include <array>
#include <cstdint>

namespace gba
{
  class ALU {
  public:
    // executeDataProcessing() takes a decoded instruction and performs the appropriate ALU operation (e.g., ADD, SUB,
    // MOV) based on the aluOpcode field.
    static void executeDataProcessing(const Instruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr);

  private:
    [[nodiscard]] static uint32_t getShiftedOperand2(const Instruction& inst);
    static void updateNZFlags(uint32_t result, uint32_t& cpsr);
  };
} // namespace gba
