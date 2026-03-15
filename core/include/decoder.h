#pragma once

#include <cstdint>

namespace gba
{
  // A simple struct to hold information about the current instruction
  struct Instruction {
    uint32_t rawOpcode; // The raw 32-bit opcode fetched from memory

    // Condition Code (Bits 28-31)
    uint8_t conditionCode;

    // Format identifier (Bits 25-27)
    uint8_t format;

    // Data Processing
    uint8_t aluOpcode; // Bits 21-24 (Specifies ADD, SUB, MOV, etc.)
    bool iBit;         // Bit 25 Immediate bit (1 = immediate operand, 0 = register operand)
    bool sBit;         // Bit 20 (Should we update CPSR flags?)
    uint8_t rn;        // Bits 16-19 First operand register
    uint8_t rd;        // Bits 12-15 Destination register

    // Immediate operand (I == 1)
    uint16_t operand2;

    // Register Operand (I == 0)
    uint8_t rm;           // Register to be shifted Bits 0-3
    bool shiftByRegister; // Whether the shift amount is specified in a register (Bit 4)
    // Logical Shift Left (LSL), Logical Shift Right (LSR), Arithmetic Shift Right (ASR), Rotate Right (ROR)
    uint8_t shiftType;   // LSL, LSR, ASR, ROR (Bits 5-6)
    uint8_t shiftAmount; // Constant shift amount (Bits 7-11)
    uint8_t rs;          // Register that contains the shift amount (Bits 8-11)

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

    // Halfword and Signed Data Transfer
    // Arm added this late when they run out of top-level instruction formats, so they had to reuse the single data
    // transfer format with some extra bits to specify the halfword/signed transfer
    bool isHalfwordTransfer;
    uint8_t hwOpcode; // For halfword and signed data transfer instructions, the specific opcode (Bits 5-6)

    // Branch and Exchange (BX) instruction
    bool isBx;
  };

  class Decoder {
  public:
    // Pure stateless function: Takes a 32-bit integer, returns a structured Instruction
    [[nodiscard]] static Instruction decodeARM(uint32_t opcode);
  };
} // namespace gba
