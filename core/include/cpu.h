#pragma once

#include <bus.h>

namespace gba
{
  class CPU {
  public:
    explicit CPU(Bus& bus_);
    ~CPU() = default;

    // reset() resets the CPU to its power-on state
    void reset();

    // step() executes a single instruction
    void step();

    // Getters for testing
    [[nodiscard]] uint32_t getRegisters(size_t index) const;
    [[nodiscard]] uint32_t getCPSR() const
    {
      return currentProgramStatusRegister;
    };

  private:
    Bus& bus;

    // R0-R15
    // R13 = Stack Pointer (SP)
    // R14 = Link Register (LR)
    // R15 = Program Counter (PC)
    std::array<uint32_t, 16> registers{};

    uint32_t currentProgramStatusRegister = 0;
  };
} // namespace gba
