#pragma once

#include <bus.h>

namespace gba
{
  // A simple struct to hold information about the current instruction
  struct Instruction {
    uint32_t opcode;
  };

  // The GBA's CPU is an ARM7TDMI, which supports both 32-bit ARM instructions and 16-bit Thumb instructions.
  //
  // The GBA has a 3-stage pipeline - Fetch, Decode and Execute. When the CPU is executing an instruction
  // at addresss 0, it is decoding the instruction at +4 and fetching the instruction at +8.
  // This means that the current Program Counter (R15) is always 8 bytes ahead of the instruction being executed in ARM
  // mode.
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

  private:
    // Pipeline stages
    void fetch();
    void decode();
    void execute();

    void executeDataProcessing(uint32_t opcode);

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
  };
} // namespace gba
