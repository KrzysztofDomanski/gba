#include <alu.h>
#include <cpu.h>
#include <lsu.h>

using namespace gba;

CPU::CPU(Bus& bus_) : bus(bus_)
{
}

void CPU::reset()
{
  // Reset all registers to 0
  registers.fill(0);

  // System Mode, ARM state, IRQ/FIQ (Interrupt Request, Fast Interrupt Request) disabled
  currentProgramStatusRegister = 0x0000001F;

  // Set the Stack Pointer (R13) to the post-BIOS IWRAM location
  registers[13] = 0x03007F00;

  // The PC (R15) points to the current instruction being FETCHED.
  // Because of the pipeline, when execution starts at 0x08000000,
  // the PC will already be at 0x08000008.
  registers[15] = 0x08000000;

  flushPipeline();
}

void CPU::step()
{
  uint32_t pcBefore = registers[15];

  execute();

  // If the PC is exactly the same, this was a normal instruction.
  // Advance the pipeline as normal.
  if (registers[15] == pcBefore) {
    decode();
    fetch();
  }

  // If the PC changed, it means we executed a jump instruction.
  // The pipeline has already been flushed in executeBranch(), so we don't need to do anything here.
}

void CPU::fetch()
{
  fetchedOpcode = bus.read32(registers[15]);
  registers[15] += 4;
}

void CPU::decode()
{
  decodedInstruction = Decoder::decodeARM(fetchedOpcode);
}

void CPU::execute()
{
  if (!checkCondition(decodedInstruction.conditionCode)) {
    return; // Condition not met, skip execution
  }

  uint32_t opcode = decodedInstruction.rawOpcode;
  uint8_t format = (opcode >> 25) & 0x7;

  switch (format) {
    case 0b000:
      if (decodedInstruction.isHalfwordTransfer) {
        LSU::executeHalfwordTransfer(decodedInstruction, registers, bus);
      } else {
        ALU::executeDataProcessing(decodedInstruction, registers, currentProgramStatusRegister);
      }
      break;
    case 0b001:
      // Bits 25-27 being 000 or 001 usually means a Data Processing instruction
      // (e.g., ADD, SUB, AND, ORR)
      ALU::executeDataProcessing(decodedInstruction, registers, currentProgramStatusRegister);
      break;
    case 0b010:
      LSU::executeSingleDataTransfer(decodedInstruction, registers, currentProgramStatusRegister, bus);
      break;
    case 0b100:
      LSU::executeBlockDataTransfer(decodedInstruction, registers, currentProgramStatusRegister, bus);
      break;
    case 0b101:
      executeBranch();
      break;
    default:
      break;
  }
}

void CPU::flushPipeline()
{
  // A helper to refill the pipeline after a jump
  fetch();
  decode();
  fetch();
}

void CPU::executeBranch()
{
  // Branch with Link (BL) is used for function calls.
  // It saves the return address into the Link Register (R14)
  if (decodedInstruction.linkBit) {
    // Because of the pipeline, PC (R15) is currently pointing to the insutrction
    // after the next one. We want to return to the instruction immediately
    // following the branch, which is PC - 4
    registers[14] = registers[15] - 4;
  }

  // Calculate target address
  int32_t byteOffset = decodedInstruction.branchOffset << 2;

  // Add offset to PC
  // The ARM spec says the offset is relative to PC + 8.
  // Since our pipeline already naturally advances R15 by 8 bytes ahead of the
  // currently executing instruction, we can just add the offset to R15 directly.
  registers[15] += byteOffset;

  // Since the PC has changed the instructions in the Fetch and Decode stages are no longer valid.
  // We need to flush the pipeline to fetch the correct instructions.
  flushPipeline();
}

bool CPU::checkCondition(uint8_t conditionCode) const
{
  // Extract the flags from the CPSR
  bool n = (currentProgramStatusRegister >> 31) & 0x1; // Negative
  bool z = (currentProgramStatusRegister >> 30) & 0x1; // Zero
  bool c = (currentProgramStatusRegister >> 29) & 0x1; // Carry
  bool v = (currentProgramStatusRegister >> 28) & 0x1; // Overflow

  switch (conditionCode) {
    case 0x0:
      return z; // EQ: Equal (Z set)
    case 0x1:
      return !z; // NE: Not Equal (Z clear)
    case 0x2:
      return c; // CS/HS: Carry Set / Unsigned Higher or Same (C set)
    case 0x3:
      return !c; // CC/LO: Carry Clear / Unsigned Lower (C clear)
    case 0x4:
      return n; // MI: Minus / Negative (N set)
    case 0x5:
      return !n; // PL: Plus / Positive or Zero (N clear)
    case 0x6:
      return v; // VS: Overflow (V set)
    case 0x7:
      return !v; // VC: No Overflow (V clear)
    case 0x8:
      return c && !z; // HI: Unsigned Higher
    case 0x9:
      return !c || z; // LS: Unsigned Lower or Same
    case 0xA:
      return n == v; // GE: Signed Greater or Equal
    case 0xB:
      return n != v; // LT: Signed Less Than
    case 0xC:
      return !z && (n == v); // GT: Signed Greater Than
    case 0xD:
      return z || (n != v); // LE: Signed Less or Equal
    case 0xE:
      return true; // AL: Always
    default:
      return false; // 0xF is reserved and should never be used
  }
}

uint32_t CPU::getRegister(size_t index) const
{
  return index >= 16 ? 0 : registers[index];
}
