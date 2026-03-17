#include <alu.h>
#include <cpu.h>
#include <lsu.h>
#include <thumb_alu.h>
#include <thumb_lsu.h>

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
  pipelineFlushed = false;
  execute();

  // If the PC is exactly the same, this was a normal instruction.
  // Advance the pipeline as normal.
  if (!pipelineFlushed) {
    decode();
    fetch();
  }
}

void CPU::fetch()
{
  // Check bit 5 of CPSR to determine the current state
  bool isThumb = currentProgramStatusRegister & 0x20;

  if (isThumb) {
    // THUMB state - fetch a 16-bit instruction
    // Advance the PC by 2 bytes to point to the next instruction
    fetchedOpcode = bus.read16(registers[15]);
    registers[15] += 2;
  } else {
    // ARM state - fetch a 32-bit instruction
    // Advance the PC by 4 bytes to point to the next instruction
    fetchedOpcode = bus.read32(registers[15]);
    registers[15] += 4;
  }
}

void CPU::decode()
{
  bool isThumb = currentProgramStatusRegister & 0x20;

  if (isThumb) {
    decodedThumbInstruction = ThumbDecoder::decode(static_cast<uint16_t>(fetchedOpcode));
  } else {
    decodedInstruction = Decoder::decodeARM(fetchedOpcode);
  }
}

void CPU::execute()
{
  bool isThumb = currentProgramStatusRegister & 0x20;
  if (isThumb) {
    switch (decodedThumbInstruction.format) {
      case 3:
        ThumbALU::executeFormat3(decodedThumbInstruction, registers, currentProgramStatusRegister);
        break;

      case 9:
        ThumbLSU::executeFormat9(decodedThumbInstruction, registers, bus);
        break;

      case 14:
        if (ThumbLSU::executeFormat14(decodedThumbInstruction, registers, bus)) {
          flushPipeline(); // PC was modified, flush the pipeline to fetch the correct instructions
        }
        break;

      case 19:
        if (!decodedThumbInstruction.isBlSuffix) {
          // Prefix
          // Sign-extend the 11-bit offset to 32 bits
          int32_t offset = decodedThumbInstruction.linkOffset;
          if (offset & 0x400) { // If the 11th bit is 1, it's negative
            offset |= 0xFFFFF800;
          }

          // Shift left by 12, add to PC, and store in LR (R14)
          // Note: The PC is currently 4 bytes ahead of this executing instruction
          registers[14] = registers[15] + (offset << 12);
        } else {
          // Suffix
          // The final target is the partial math in LR + (lower offset shifted left by 1)
          uint32_t targetAddress = registers[14] + (decodedThumbInstruction.linkOffset << 1);

          // The return address is the instruction immediately following this suffix
          // Because PC is currently 4 bytes ahead of the suffix, we subtract 2
          // THUMB specification requires bit 0 to be set to 1 to indicate THUMB state
          registers[14] = (registers[15] - 2) | 1;

          // Jump to the target address
          registers[15] = targetAddress;
          flushPipeline();
        }
        break;

      case 16:
        if (checkCondition(decodedThumbInstruction.cond)) {
          // THUMB branch offsets are 11 bits and represent multiples of 2 bytes
          int32_t byteOffset = decodedThumbInstruction.branchOffset << 1;

          registers[15] += byteOffset;

          // PC has changed so we need to flush the pipeline to fetch the correct instructions
          flushPipeline();
        }
        break;
      default:
        break; // Unimplemented THUMB formats do nothing for now
    }

    // Return early since we don't want to execute ARM instructions when in THUMB state
    return;
  }

  if (!checkCondition(decodedInstruction.conditionCode)) {
    return; // Condition not met, skip execution
  }

  // Handle branch and exchange
  if (decodedInstruction.isBx) {
    uint32_t targetAddress = registers[decodedInstruction.rm];

    // Check the lowest bit to determie the new state
    if (targetAddress & 1) {
      // Bit 5 set = THUMB state
      currentProgramStatusRegister |= 0x20; // Set bit 5 to switch to THUMB state
    } else {
      // Bit 5 clear = ARM state
      currentProgramStatusRegister &= ~0x20; // Clear bit 5 to switch to ARM state
    }

    // Clear the lowest bit to get the actual address
    registers[15] = targetAddress & ~1;
    flushPipeline();
    return;
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
  pipelineFlushed = true;
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
