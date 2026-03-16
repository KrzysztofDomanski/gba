#pragma once

#include <cstdint>

namespace gba
{
  struct ThumbInstruction {
    uint16_t rawOpcode;

    // THUMB instruction format
    // THUMB has 19 distinct instruction formats
    uint8_t format;

    // Common fields
    uint8_t rd;        // Destination register
    uint8_t rs;        // Source register
    uint8_t rn;        // Second source register (if applicable)
    uint8_t immediate; // Immediate value (if applicable)
    uint8_t opcode;    // Sub opcode for math ops (if applicable)

    // Branching
    uint8_t cond;
    uint32_t branchOffset;

    // Load/Store
    uint8_t rb; // Base register for load/store (Bits 3-5)
    bool bBit;  // Byte/Word flag
    bool lBit;  // Load/Store flag
  };

  class ThumbDecoder {
  public:
    /// decode() takes a raw 16-bit opcode and decodes it into a ThumbInstruction struct.
    [[nodiscard]] static ThumbInstruction decode(uint16_t rawOpcode);
  };
} // namespace gba
