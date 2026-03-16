include(FetchContent)

FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(catch2)

add_executable(gba_tests
  test_block_transfer.cxx
  test_branch.cxx
  test_bus.cxx
  test_bx.cxx
  test_cartridge.cxx
  test_conditions.cxx
  test_cpu.cxx
  test_halfword.cxx
  test_loop.cxx
  test_lsu.cxx
  test_main.cxx
  test_ppu.cxx
  test_shifts.cxx
  test_thumb_decode.cxx
  test_vram.cxx
)
target_link_libraries(gba_tests PRIVATE gba_core Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(gba_tests)
