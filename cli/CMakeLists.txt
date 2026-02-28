add_executable(gba_cli main.cxx)

# Link to the core C++ library
target_link_libraries(gba_cli PRIVATE gba_core)
