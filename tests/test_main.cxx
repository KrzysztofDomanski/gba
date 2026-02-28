#include <catch2/catch_test_macros.hpp>

#include <emulator.h>

TEST_CASE("Emulator initializes successfully", "[emulator]") {
  gba::Emulator emulator;
  REQUIRE(emulator.init() == true);
}
