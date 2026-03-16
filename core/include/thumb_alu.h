#pragma once

#include <thumb_decoder.h>

#include <array>
#include <cstdint>

namespace gba
{
  class ThumbALU {
  public:
    /// executeFormat3() executes THUMB format 3 instructions, which are MOV, CMP, ADD and SUB with immediate values.
    static void executeFormat3(const ThumbInstruction& inst, std::array<uint32_t, 16>& registers, uint32_t& cpsr);

  private:
    static void updateNZFlags(uint32_t result, uint32_t& cpsr);
  };
} // namespace gba
