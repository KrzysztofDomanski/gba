#pragma once

#include <cstdint>

namespace gba
{
  // A simple struct to hold information about the current instruction
  struct Instruction {
    uint32_t rawOpcode; // The raw 32-bit opcode fetched from memory
    uint8_t aluOpcode;  // Bits 21-24 (Specifies ADD, SUB, MOV, etc.)
    bool iBit;          // Bit 25 Immediate bit (1 = immediate operand, 0 = register operand)
    bool sBit;          // Bit 20 (Should we update CPSR flags?)
    uint8_t rn;         // Bits 16-19 First operand register
    uint8_t rd;         // Bits 12-15 Destination register
    uint16_t operand2;  // Bits 0-11 Second operand (immediate or register)
  };

  class Decoder {
  public:
    // Pure stateless function: Takes a 32-bit integer, returns a structured Instruction
    [[nodiscard]] static Instruction decodeARM(uint32_t opcode);
  };
} // namespace gba
