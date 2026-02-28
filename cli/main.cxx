#include <iostream>

#include <emulator.h>

int main() {
  std::cout << "Starting GBA Emulator CLI Runner..." << std::endl;
  gba::Emulator emulator;
  if (emulator.init()) {
    std::cout << "Emulator initialized successfully!" << std::endl;
  } else {
    std::cerr << "Failed to initialize the emulator." << std::endl;
    return 1;
  }

  return 0;
}
