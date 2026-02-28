include(FetchContent)

FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(catch2)

add_executable(gba_tests test_main.cxx)
target_link_libraries(gba_tests PRIVATE gba_core Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(gba_tests)
