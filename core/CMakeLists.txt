add_library(gba_core STATIC
  src/alu.cxx
  src/bus.cxx
  src/cartridge.cxx
  src/cpu.cxx
  src/decoder.cxx
  src/emulator.cxx
  src/lsu.cxx
  src/ppu.cxx
  src/thumb_alu.cxx
  src/thumb_decoder.cxx
  src/thumb_lsu.cxx
)
target_include_directories(gba_core PUBLIC include)
