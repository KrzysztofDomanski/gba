#include <cartridge.h>

#include <fstream>
#include <iostream>

using namespace gba;

bool Cartridge::load(const std::filesystem::path& path)
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    std::cerr << "Failed to open ROM file: " << path << std::endl;
    return false;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  if (size > 32 * 1024 * 1024) {
    std::cerr << "ROM file is too large: " << size << " bytes" << std::endl;
    return false;
  }

  rom.resize(static_cast<size_t>(size));
  if (file.read(reinterpret_cast<char*>(rom.data()), size)) {
    std::cout << "Successfully loaded ROM: " << path << " (" << size << " bytes)" << std::endl;
    return true;
  }

  return false;
}

uint8_t Cartridge::read8(uint32_t address) const
{
  if (rom.empty() || address >= rom.size()) {
    return 0x00;
  }

  return rom[address];
}
