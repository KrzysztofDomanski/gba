add_library(gba_core STATIC src/emulator.cxx src/bus.cxx)
target_include_directories(gba_core PUBLIC include)
