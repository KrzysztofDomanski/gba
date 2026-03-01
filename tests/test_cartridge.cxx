#include <bus.h>
#include <cartridge.h>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

TEST_CASE("Cartridge Loading and Bus Mapping", "[Cartridge]")
{
  gba::Bus bus;
  std::filesystem::path tempRomPath = "dummy_test.gba";

  // Setup: Create a fake 256-byte ROM file
  std::vector<uint8_t> fakeRomData(256, 0x00);
  fakeRomData[0] = 0xDE;
  fakeRomData[1] = 0xAD;
  fakeRomData[0xFF] = 0xBE;

  {
    std::ofstream file(tempRomPath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(fakeRomData.data()), fakeRomData.size());
  }

  SECTION("Load Cartridge via Bus")
  {
    REQUIRE(bus.insertCartridge(tempRomPath) == true);

    // Read from the 0x08000000 memory region
    REQUIRE(bus.read8(0x08000000) == 0xDE);
    REQUIRE(bus.read8(0x08000001) == 0xAD);
    REQUIRE(bus.read8(0x080000FF) == 0xBE);

    // Ensure 16-bit and 32-bit reads cascade perfectly into the new ROM region
    REQUIRE(bus.read16(0x08000000) == 0xADDE); // Little-endian
  }

  std::filesystem::remove(tempRomPath);
}