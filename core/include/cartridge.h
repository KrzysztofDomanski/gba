#pragma once

#include <filesystem>
#include <vector>

namespace gba
{
  /// Represents a GBA Rom
  ///
  /// A GBA ROM can be up to 32MB in size, I used a std::vector
  /// and I will allocated the exact size needed.
  class Cartridge {
  public:
    Cartridge() = default;
    ~Cartridge() = default;

    [[nodiscard]] bool load(const std::filesystem::path& romPath);

    [[nodiscard]] uint8_t read8(uint32_t address) const;

    [[nodiscard]] size_t getSize() const
    {
      return rom.size();
    }

    [[nodiscard]] bool isLoaded() const
    {
      return !rom.empty();
    }

  private:
    std::vector<uint8_t> rom;
  };
} // namespace gba
