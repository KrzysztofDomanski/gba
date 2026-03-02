#pragma once

#include <cstdint>

namespace gba
{
  // A simple struct to hold information about the current instruction
  struct Instruction {
    uint32_t rawOpcode; // The raw 32-bit opcode fetched from memory

    // Format identifier (Bits 25-27)
    uint8_t format;

    // Data Processing
    uint8_t aluOpcode; // Bits 21-24 (Specifies ADD, SUB, MOV, etc.)
    bool iBit;         // Bit 25 Immediate bit (1 = immediate operand, 0 = register operand)
    bool sBit;         // Bit 20 (Should we update CPSR flags?)
    uint8_t rn;        // Bits 16-19 First operand register
    uint8_t rd;        // Bits 12-15 Destination register
    uint16_t operand2; // Bits 0-11 Second operand (immediate or register)

    // Branch
    int32_t branchOffset; // For branch instructions, the signed offset to apply to the PC
    bool linkBit;         // For branch instructions, whether to set the Link Register (LR) to the return address

    // Load/Store (Single Data Transfer)
    bool pBit;             // Pre/Post indexing bit (Bit 24)
    bool uBit;             // Up/Down bit (Bit 23)
    bool bBit;             // Byte/Word bit (Bit 22)
    bool wBit;             // Write-back bit (Bit 21)
    bool lBit;             // Load/Store bit (Bit 20)
    uint16_t memoryOffset; // 12-bit immediate offset

    // Block Data Transfer
    uint16_t registerList; // For block data transfer, a bitmask of registers to load/store (Bits 0-15)
  };

  class Decoder {
  public:
    // Pure stateless function: Takes a 32-bit integer, returns a structured Instruction
    [[nodiscard]] static Instruction decodeARM(uint32_t opcode);
  };
} // namespace gba
