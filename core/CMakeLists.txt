add_library(gba_core STATIC
  src/bus.cxx
  src/cartridge.cxx
  src/emulator.cxx
)
target_include_directories(gba_core PUBLIC include)
