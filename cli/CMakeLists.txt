add_executable(gba_cli
  main.cxx
  src/display.cxx
)

include(FetchContent)
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.0
)
FetchContent_MakeAvailable(SDL2)

# Link to the core C++ library
target_link_libraries(gba_cli PRIVATE gba_core SDL2::SDL2-static)
target_include_directories(gba_cli PRIVATE include)
