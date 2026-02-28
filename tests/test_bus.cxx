#include <bus.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Bus Read/Write Operations", "[Bus]") {
  gba::Bus bus;

  SECTION("EWRAM 8-bit Read/Write") {
    uint32_t address = 0x02000000; // Start of EWRAM
    bus.write8(address, 0xAB);
    REQUIRE(bus.read8(address) == 0xAB);
  }

  SECTION("IWRAM 8-bit Read/Write") {
    uint32_t address = 0x03000000; // Start of IWRAM
    bus.write8(address, 0xCD);
    REQUIRE(bus.read8(address) == 0xCD);
  }

  SECTION("EWRAM 16-bit Read/Write") {
    uint32_t address = 0x02000000; // Start of EWRAM
    bus.write16(address, 0x1234);
    REQUIRE(bus.read16(address) == 0x1234);
  }

  SECTION("IWRAM 16-bit Read/Write") {
    uint32_t address = 0x03000000; // Start of IWRAM
    bus.write16(address, 0x5678);
    REQUIRE(bus.read16(address) == 0x5678);
  }

  SECTION("EWRAM 32-bit Read/Write") {
    uint32_t address = 0x02000000; // Start of EWRAM
    bus.write32(address, 0xDEADBEEF);
    REQUIRE(bus.read32(address) == 0xDEADBEEF);

    // Verify little-endian storage
    REQUIRE(bus.read8(address) == 0xEF);
    REQUIRE(bus.read8(address + 1) == 0xBE);
    REQUIRE(bus.read8(address + 2) == 0xAD);
    REQUIRE(bus.read8(address + 3) == 0xDE);
  }

  SECTION("Address Alignment Enforcement") {
    uint32_t address = 0x02000001; // Unaligned address for 16-bit access
    bus.write16(address, 0x1234);
    // Should align down to 0x02000000
    REQUIRE(bus.read16(address & ~1) == 0x1234);

    address = 0x02000002; // Unaligned address for 32-bit access
    bus.write32(address, 0xDEADBEEF);
    // Should align down to 0x02000000
    REQUIRE(bus.read32(address & ~3) == 0xDEADBEEF);
  }
}