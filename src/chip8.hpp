#pragma once
#include <cstdint>
#include <string>
#include <random>
#include <array>

class Chip8 {
  public:
    static constexpr unsigned int VIDEO_WIDTH = 64;
    static constexpr unsigned int VIDEO_HEIGHT = 32;
    uint8_t delayTimer = 0; // Delay timer
    uint8_t soundTimer = 0; // Sound timer

    Chip8();

    void Initialize();
    bool LoadROM(const std::string& filePath);

    void Cycle();

    const std::array<uint8_t, VIDEO_WIDTH * VIDEO_HEIGHT>& GetVideo() const {
      return video;
    }
    
    void SetKeyState(uint8_t key, bool pressed);
  
  private:
  std::array<uint8_t, 4096> memory{}; // 4K memory
  std::array<uint8_t, 16> V{}; // 16 registers V0 to VF
  uint16_t I = 0; // Register I
  uint16_t pc = 0; // Program counter

  std::array<uint16_t, 16> stack{}; // Stack with 16 levels
  uint8_t sp = 0; // Stack pointer

  std::array<uint8_t, 16> keypad{}; // Keypad state of 0x0 to 0xF
  std::array<uint8_t, VIDEO_WIDTH * VIDEO_HEIGHT> video{}; // 0 o 1 for each pixel

  uint16_t opcode = 0;

  std::mt19937 randGen; // Random number generator
  std::uniform_int_distribution<uint8_t> randByte; // Distribution for random byte

  void LoadFontset();
};
