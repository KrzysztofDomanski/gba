#include <iostream>

#include <bus.h>
#include <cpu.h>
#include <ppu.h>

#include <display.h>

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_gba_rom>" << std::endl;
    return EXIT_FAILURE;
  }

  std::filesystem::path romPath = argv[1];

  gba::Bus bus;
  gba::CPU cpu(bus);
  gba::PPU ppu(bus);
  gba::cli::Display display;

  if (!bus.insertCartridge(romPath)) {
    std::cerr << "Failed to load cartridge: " << romPath << std::endl;
    return EXIT_FAILURE;
  }

  if (!display.initialize()) {
    std::cerr << "Failed to initialize display." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Booting up Game Boy Advance Emulator..." << std::endl;
  cpu.reset();

  bool isRunning = true;

  const int STEPS_PER_FRAME = 280000; // Approximate number of CPU cycles per frame at 16.78 MHz

  while (isRunning) {
    isRunning = display.processEvents();

    for (int i = 0; i < STEPS_PER_FRAME; ++i) {
      cpu.step();
    }

    ppu.renderFrame();

    display.render(ppu.getFrameBuffer());
  }

  std::cout << "Shutting down emulator..." << std::endl;
  return EXIT_SUCCESS;
}
