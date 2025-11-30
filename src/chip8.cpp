#include "chip8.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

static const uint8_t CHIP8_FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() : randGen(std::random_device{}()), randByte(0, 255)
{
  Initialize();
}

void Chip8::Initialize()
{
  memory.fill(0);
  V.fill(0);
  stack.fill(0);
  keypad.fill(0);
  video.fill(0);

  I = 0;
  pc = 0x200; // Programs start at 0x200
  sp = 0;

  delayTimer = 0;
  soundTimer = 0;

  opcode = 0;
  LoadFontset();
}

void Chip8::LoadFontset()
{
  const uint16_t fontStart = 0x000; // Fontset starts at memory location 0x000
  std::memcpy(&memory[fontStart], CHIP8_FONTSET, sizeof(CHIP8_FONTSET));
}

bool Chip8::LoadROM(const std::string &filePath)
{
  std::ifstream rom(filePath, std::ios::binary | std::ios::ate);
  if (!rom)
  {
    std::cerr << "Failed to open ROM: " << filePath << std::endl;
    return false;
  }

  std::streamsize size = rom.tellg();
  rom.seekg(0, std::ios::beg);

  if (size > (4096 - 0x200))
  {
    std::cerr << "ROM size exceeds available memory." << std::endl;
    return false;
  }

  std::vector<char> buffer(size);
  if (!rom.read(buffer.data(), size))
  {
    std::cerr << "Failed to read ROM data." << std::endl;
    return false;
  }

  for (size_t i = 0; i < static_cast<size_t>(size); i++)
  {
    memory[0x200 + i] = static_cast<uint8_t>(buffer[i]);
  }

  std::cout << "ROM loaded successfully: " << filePath << std::endl;
  return true;
}

void Chip8::SetKeyState(uint8_t key, bool pressed)
{
  if (key < 16)
  {
    keypad[key] = pressed ? 1 : 0;
  }
}

void Chip8::Cycle()
{
  // FETCH
  opcode = (memory[pc] << 8) | memory[pc + 1];
  pc += 2;

  uint16_t nnn = opcode & 0x0FFF;
  uint8_t nn = opcode & 0x00FF;
  uint8_t n = opcode & 0x000F;
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  switch (opcode & 0xF000)
  {
  case 0x0000:
    switch (opcode)
    {
    case 0x00E0: // CLS
      video.fill(0);
      break;

    case 0x00EE: // RET
      sp--;
      pc = stack[sp];
      break;

    default:
      std::cerr << "Unknown 0x0000 opcode: "
                << std::hex << opcode << "\n";
      break;
    }
    break;

  case 0x1000: // JP NNN
    pc = nnn;
    break;

  case 0x2000: // CALL NNN
    stack[sp] = pc;
    sp++;
    pc = nnn;
    break;

  case 0x3000: // SE Vx, NN
    if (V[x] == nn)
      pc += 2;
    break;

  case 0x4000: // SNE Vx, NN
    if (V[x] != nn)
      pc += 2;
    break;

  case 0x5000: // SE Vx, Vy
    if (V[x] == V[y])
      pc += 2;
    break;

  case 0x6000: // LD Vx, NN
    V[x] = nn;
    break;

  case 0x7000: // ADD Vx, NN
    V[x] += nn;
    break;

  case 0x8000:
    switch (opcode & 0x000F)
    {
    case 0x0:
      V[x] = V[y];
      break; // LD Vx, Vy
    case 0x1:
      V[x] |= V[y];
      break; // OR
    case 0x2:
      V[x] &= V[y];
      break; // AND
    case 0x3:
      V[x] ^= V[y];
      break; // XOR
    case 0x4:
    { // ADD + carry
      uint16_t sum = V[x] + V[y];
      V[0xF] = (sum > 255);
      V[x] = sum & 0xFF;
    }
    break;
    case 0x5:
    { // SUB
      V[0xF] = (V[x] > V[y]);
      V[x] -= V[y];
    }
    break;
    case 0x6: // SHR
      V[0xF] = V[x] & 1;
      V[x] >>= 1;
      break;
    case 0x7:
    { // SUBN
      V[0xF] = (V[y] > V[x]);
      V[x] = V[y] - V[x];
    }
    break;
    case 0xE: // SHL
      V[0xF] = (V[x] >> 7) & 1;
      V[x] <<= 1;
      break;

    default:
      std::cerr << "Unknown 0x8XY? opcode: "
                << std::hex << opcode << "\n";
    }
    break;

  case 0x9000: // SNE Vx, Vy
    if (V[x] != V[y])
      pc += 2;
    break;

  case 0xA000: // LD I, NNN
    I = nnn;
    break;

  case 0xB000: // JP V0 + addr
    pc = V[0] + nnn;
    break;

  case 0xC000: // CXNN: RND Vx, NN
    V[x] = randByte(randGen) & nn;
    break;

  case 0xD000:
  { // DXYN: DRW (draw sprite)
    uint8_t width = 8;
    uint8_t height = n;

    V[0xF] = 0;

    for (unsigned int row = 0; row < height; row++)
    {
      uint8_t pixel = memory[I + row];

      for (unsigned int col = 0; col < width; col++)
      {
        if (pixel & (0x80 >> col))
        {
          uint16_t px = V[x] + col;
          uint16_t py = V[y] + row;

          // SOLO dibujar si está dentro de los límites
          if (px < VIDEO_WIDTH && py < VIDEO_HEIGHT)
          {
            uint16_t index = py * VIDEO_WIDTH + px;

            if (video[index] == 1)
              V[0xF] = 1; // colisión

            video[index] ^= 1; // toggle pixel
          }
        }
      }
    }
  }
  break;

  case 0xE000:
    switch (opcode & 0x00FF)
    {
    case 0x9E: // SKP Vx
      if (keypad[V[x]])
        pc += 2;
      break;

    case 0xA1: // SKNP Vx
      if (!keypad[V[x]])
        pc += 2;
      break;

    default:
      std::cerr << "Unknown EX?? opcode: "
                << std::hex << opcode << "\n";
      break;
    }
    break;

  case 0xF000:
    switch (opcode & 0x00FF)
    {
    case 0x07:
      V[x] = delayTimer;
      break;
    case 0x15:
      delayTimer = V[x];
      break;
    case 0x18:
      soundTimer = V[x];
      break;

    case 0x1E:
      I += V[x];
      break;

    case 0x0A: // LD Vx, key
      // Esto requiere pausa hasta que se presione una tecla
      for (uint8_t i = 0; i < 16; i++)
      {
        if (keypad[i])
        {
          V[x] = i;
          return;
        }
      }
      pc -= 2; // repetir instrucción
      break;

    case 0x29: // LD F, Vx → font sprite
      I = 0x000 + (V[x] * 5);
      break;

    case 0x33: // BCD
      memory[I] = V[x] / 100;
      memory[I + 1] = (V[x] / 10) % 10;
      memory[I + 2] = V[x] % 10;
      break;

    case 0x55: // store V0..Vx en memoria
      for (uint8_t i = 0; i <= x; i++)
        memory[I + i] = V[i];
      break;

    case 0x65: // load V0..Vx desde memoria
      for (uint8_t i = 0; i <= x; i++)
        V[i] = memory[I + i];
      break;

    default:
      std::cerr << "Unknown FX?? opcode: "
                << std::hex << opcode << "\n";
      break;
    }
    break;

  default:
    std::cerr << "Unknown opcode: " << std::hex << opcode << "\n";
    break;
  }

  // Update timers
  if (delayTimer > 0)
    delayTimer--;
  if (soundTimer > 0)
    soundTimer--;
}
