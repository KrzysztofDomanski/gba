#include <bus.h>
#include <catch2/catch_test_macros.hpp>
#include <cpu.h>
#include <filesystem>
#include <fstream>

TEST_CASE("CPU Branch and Link", "[cpu][branch]")
{
  gba::Bus bus;
  std::filesystem::path tempRomPath = "test_branch.gba";

  // Assembly Program:
  // 0x08000000: B skip      (Branch forward by 1 instruction)  -> 0xEA000000
  // 0x08000004: MOV R0, #5  (This should be SKIPPED)           -> 0xE3A00005
  // 0x08000008: MOV R1, #10 (skip label - execution resumes)   -> 0xE3A0100A
  std::vector<uint32_t> instructions = {0xEA000000, 0xE3A00005, 0xE3A0100A};

  {
    std::ofstream file(tempRomPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(tempRomPath) == true);
  gba::CPU cpu(bus);

  SECTION("Execute Branch and Flush Pipeline")
  {
    cpu.reset();

    // Execute the Branch instruction
    // This will modify R15 to 0x08000008 and flush the pipeline
    cpu.step();

    // R0 should stilll be 0 because of the MOV R0, #5 instruction being skipped
    REQUIRE(cpu.getRegister(0) == 0);

    // Execute the instruction at the new destination (MOV R1, #10)
    cpu.step();
    REQUIRE(cpu.getRegister(1) == 10);
  }

  std::filesystem::remove(tempRomPath);
}