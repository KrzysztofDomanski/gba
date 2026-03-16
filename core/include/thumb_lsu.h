#pragma once

#include <bus.h>
#include <thumb_decoder.h>

#include <array>
#include <cstdint>

namespace gba
{
  class ThumbLSU {
  public:
    /// executeFormat9() executes THUMB format 9 load/store instructions.
    static void executeFormat9(const ThumbInstruction& inst, std::array<uint32_t, 16>& registers, Bus& bus);

    /// executeFormat14() executes THUMB format 14 push/pop instructions. Returns true if the
    /// PC was modified - i.e. the pipeline needs flushing
    static bool executeFormat14(const ThumbInstruction& inst, std::array<uint32_t, 16>& registers, Bus& bus);
  };
} // namespace gba