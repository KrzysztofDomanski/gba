#include <catch2/catch_test_macros.hpp>

#include <bus.h>
#include <cpu.h>

#include <filesystem>
#include <fstream>

TEST_CASE("CPU Thumb Format 14 Push/Pop", "[cpu][thumb][stack]")
{
  gba::Bus bus;
  std::filesystem::path temp_rom_path = "test_thumb_stack.gba";

  std::vector<uint32_t> instructions = {
      0xE3A0D403, // ARM: MOV R13, #0x03000000
      0xE28F0001, // ARM: ADD R0, PC, #1
      0xE12FFF10, // ARM: BX R0
      0x2114200A, // Packed THUMB: MOV R1, #20 (Upper) | MOV R0, #10 (Lower)
      0xBC0CB403  // Packed THUMB: POP {R2, R3} (Upper) | PUSH {R0, R1} (Lower)
  };

  {
    std::ofstream file(temp_rom_path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(instructions.data()), instructions.size() * sizeof(uint32_t));
  }

  REQUIRE(bus.insertCartridge(temp_rom_path) == true);
  gba::CPU cpu(bus);

  SECTION("Execute THUMB Push and Pop")
  {
    cpu.reset();

    cpu.step(); // MOV R13, #0x03000000
    cpu.step(); // ADD R0, PC, #1
    cpu.step(); // BX R0 - should switch to THUMB state and jump to 0x08000008

    REQUIRE((cpu.getCPSR() & 0x20) != 0);       // Check THUMB state bit is set
    REQUIRE(cpu.getRegister(13) == 0x03000000); // Check SP is set correctly

    cpu.step(); // MOV R0, #10 (Lower)
    cpu.step(); // MOV R1, #20 (Upper)
    REQUIRE(cpu.getRegister(0) == 10);
    REQUIRE(cpu.getRegister(1) == 20);

    cpu.step(); // PUSH {R0, R1}
    REQUIRE(cpu.getRegister(13) == 0x02FFFFF8);

    REQUIRE(bus.read32(0x02FFFFF8) == 10);
    REQUIRE(bus.read32(0x02FFFFFC) == 20);

    cpu.step(); // THUMB: POP {R2, R3}
    REQUIRE(cpu.getRegister(13) == 0x03000000);
    REQUIRE(cpu.getRegister(2) == 10);
    REQUIRE(cpu.getRegister(3) == 20);
  }

  std::filesystem::remove(temp_rom_path);
}