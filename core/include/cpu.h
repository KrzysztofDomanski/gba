#pragma once

#include <bus.h>
#include <decoder.h>
#include <thumb_decoder.h>

#include <array>
#include <bit>
#include <cstdint>

namespace gba
{

  // The GBA's CPU is an ARM7TDMI, which supports both 32-bit ARM instructions and 16-bit Thumb instructions.
  //
  // The GBA has a 3-stage pipeline - Fetch, Decode and Execute. When the CPU is executing an instruction
  // at addresss 0, it is decoding the instruction at +4 and fetching the instruction at +8.
  // This means that the current Program Counter (R15) is always 8 bytes ahead of the instruction being executed in ARM
  // mode.
  //
  // When an instruction ends with an 'S' (e.g. ADDS), it means that the instruction should update the CPSR flags based
  // on the result of the operation: N (negative), Z (zero), C (carry) and V (overflow).
  // ARM's "Operand 2" uses something called a Barrel Shifter to rotate immediate values.
  class CPU {
  public:
    explicit CPU(Bus& bus_);
    ~CPU() = default;

    // reset() resets the CPU to its power-on state
    void reset();

    // step() executes a single instruction
    void step();

    // Getters for testing
    [[nodiscard]] uint32_t getRegister(size_t index) const;
    [[nodiscard]] uint32_t getCPSR() const
    {
      return currentProgramStatusRegister;
    };

    // For testing
    void setRegister(size_t index, uint32_t value)
    {
      if (index < 16) {
        registers[index] = value;
      }
    }

  private:
    // Pipeline stages
    void fetch();
    void decode();
    void execute();
    void executeBranch();
    void flushPipeline();

    [[nodiscard]] bool checkCondition(uint8_t conditionCode) const;

    Bus& bus;

    // R0-R15
    // R13 = Stack Pointer (SP)
    // R14 = Link Register (LR)
    // R15 = Program Counter (PC)
    std::array<uint32_t, 16> registers{};
    uint32_t currentProgramStatusRegister = 0;

    // Pipeline state
    uint32_t fetchedOpcode = 0;
    Instruction decodedInstruction;
    ThumbInstruction decodedThumbInstruction;
    bool pipelineFlushed = false;
  };
} // namespace gba
