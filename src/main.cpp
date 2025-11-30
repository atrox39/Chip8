#include "chip8.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_syswm.h>
#include <windows.h>
#include <commdlg.h>

static const int WINDOW_SCALE = 10;

// --------------------- helpers globales ---------------------
const char *BaseFileName(const char *path)
{
  const char *base = strrchr(path, '\\');
  return base ? base + 1 : path;
}

// WndProc original de SDL (para no romperla)
static WNDPROC g_originalWndProc = nullptr;
// Menú de velocidad para poder marcar la palomita
static HMENU g_hSpeedMenu = nullptr;

// --------------------- WndProc personalizada ----------------
LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_COMMAND:
    // reenviamos WM_COMMAND al loop principal (PeekMessage)
    PostMessage(hwnd, msg, wParam, lParam);
    return 0;
  }

  // delegamos TODO lo demás a la WndProc original de SDL
  if (g_originalWndProc)
  {
    return CallWindowProc(g_originalWndProc, hwnd, msg, wParam, lParam);
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================
//                            MAIN
// ============================================================
int main(int argc, char **argv)
{
  int cpuSpeedHz = 500;
  bool unlimitedSpeed = false;

  auto lastCpu = std::chrono::high_resolution_clock::now();
  auto lastTimer = std::chrono::high_resolution_clock::now();

  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <ROM file>" << std::endl;
    return 1;
  }

  Chip8 chip8;
  if (!chip8.LoadROM(argv[1]))
  {
    return 1;
  }

  SDL_Init(SDL_INIT_VIDEO);

  std::string initialTitle = "CHIP-8 Emulador - " + std::string(BaseFileName(argv[1]));
  SDL_Window *window = SDL_CreateWindow(
      initialTitle.c_str(),
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      Chip8::VIDEO_WIDTH * WINDOW_SCALE,
      Chip8::VIDEO_HEIGHT * WINDOW_SCALE + 22,
      SDL_WINDOW_SHOWN);

  // Obtener HWND y subclasificar WndProc
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  HWND hwnd = wmInfo.info.win.window;

  g_originalWndProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
  SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)CustomWndProc);

  // --------------------- Menús ---------------------
  HMENU hMenu = CreateMenu();
  HMENU hFileMenu = CreatePopupMenu();
  HMENU hSpeedMenu = CreatePopupMenu();

  AppendMenu(hFileMenu, MF_STRING, 1001, TEXT("Open ROM"));
  AppendMenu(hFileMenu, MF_STRING, 1002, TEXT("Exit"));
  AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, TEXT("File"));

  // Speed (radio items)
  AppendMenu(hSpeedMenu, MF_STRING, 2001, TEXT("200 Hz"));
  AppendMenu(hSpeedMenu, MF_STRING, 2002, TEXT("300 Hz"));
  AppendMenu(hSpeedMenu, MF_STRING, 2003, TEXT("500 Hz"));
  AppendMenu(hSpeedMenu, MF_STRING, 2004, TEXT("700 Hz"));
  AppendMenu(hSpeedMenu, MF_STRING, 2005, TEXT("1000 Hz"));
  AppendMenu(hSpeedMenu, MF_STRING, 2006, TEXT("1500 Hz"));
  AppendMenu(hSpeedMenu, MF_STRING, 2007, TEXT("Max (Unlimited)"));
  AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSpeedMenu, TEXT("Speed"));

  SetMenu(hwnd, hMenu);
  DrawMenuBar(hwnd);

  g_hSpeedMenu = hSpeedMenu;

  // marcar 500 Hz por defecto
  CheckMenuRadioItem(g_hSpeedMenu, 2001, 2007, 2003, MF_BYCOMMAND);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  bool running = true;
  SDL_Event event;

  auto lastTimerUpdate = std::chrono::high_resolution_clock::now();

  while (running)
  {
    // ------------ procesar menú (WM_COMMAND) ------------
    MSG msg;
    while (PeekMessage(&msg, hwnd, WM_COMMAND, WM_COMMAND, PM_REMOVE))
    {
      if (msg.message == WM_COMMAND)
      {
        switch (msg.wParam)
        {
        // Open ROM
        case 1001:
        {
          OPENFILENAMEA ofn;
          char fileName[MAX_PATH] = {0};
          ZeroMemory(&ofn, sizeof(ofn));

          ofn.lStructSize = sizeof(ofn);
          ofn.hwndOwner = hwnd;
          ofn.lpstrFilter = "CHIP-8 ROMs\0*.ch8;*.c8\0All files\0*.*\0";
          ofn.lpstrFile = fileName;
          ofn.nMaxFile = MAX_PATH;
          ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
          ofn.lpstrDefExt = "ch8";

          if (GetOpenFileNameA(&ofn))
          {
            chip8.Initialize();
            if (!chip8.LoadROM(fileName))
            {
              std::string title = "CHIP-8 Emulador - Failed: " +
                                  std::string(BaseFileName(fileName));
              SDL_SetWindowTitle(window, title.c_str());
              MessageBoxA(hwnd, "Failed to load ROM.", "Error",
                          MB_OK | MB_ICONERROR);
            }
            else
            {
              std::string title = "CHIP-8 Emulador - " +
                                  std::string(BaseFileName(fileName));
              SDL_SetWindowTitle(window, title.c_str());
            }
          }
          break;
        }
        // Exit
        case 1002:
          running = false;
          break;

        // Speed (poner palomita y actualizar velocidad)
        case 2001:
          cpuSpeedHz = 200;
          unlimitedSpeed = false;
          CheckMenuRadioItem(g_hSpeedMenu, 2001, 2007, 2001, MF_BYCOMMAND);
          break;
        case 2002:
          cpuSpeedHz = 300;
          unlimitedSpeed = false;
          CheckMenuRadioItem(g_hSpeedMenu, 2001, 2007, 2002, MF_BYCOMMAND);
          break;
        case 2003:
          cpuSpeedHz = 500;
          unlimitedSpeed = false;
          CheckMenuRadioItem(g_hSpeedMenu, 2001, 2007, 2003, MF_BYCOMMAND);
          break;
        case 2004:
          cpuSpeedHz = 700;
          unlimitedSpeed = false;
          CheckMenuRadioItem(g_hSpeedMenu, 2001, 2007, 2004, MF_BYCOMMAND);
          break;
        case 2005:
          cpuSpeedHz = 1000;
          unlimitedSpeed = false;
          CheckMenuRadioItem(g_hSpeedMenu, 2001, 2007, 2005, MF_BYCOMMAND);
          break;
        case 2006:
          cpuSpeedHz = 1500;
          unlimitedSpeed = false;
          CheckMenuRadioItem(g_hSpeedMenu, 2001, 2007, 2006, MF_BYCOMMAND);
          break;
        case 2007:
          unlimitedSpeed = true;
          CheckMenuRadioItem(g_hSpeedMenu, 2001, 2007, 2007, MF_BYCOMMAND);
          break;
        }
      }
    }

    // ------------ eventos SDL ------------
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        running = false;
      }
      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      {
        bool pressed = (event.type == SDL_KEYDOWN);
        switch (event.key.keysym.sym)
        {
        case SDLK_1:
          chip8.SetKeyState(0x1, pressed);
          break;
        case SDLK_2:
          chip8.SetKeyState(0x2, pressed);
          break;
        case SDLK_3:
          chip8.SetKeyState(0x3, pressed);
          break;
        case SDLK_4:
          chip8.SetKeyState(0xC, pressed);
          break;

        case SDLK_q:
          chip8.SetKeyState(0x4, pressed);
          break;
        case SDLK_w:
          chip8.SetKeyState(0x5, pressed);
          break;
        case SDLK_e:
          chip8.SetKeyState(0x6, pressed);
          break;
        case SDLK_r:
          chip8.SetKeyState(0xD, pressed);
          break;

        case SDLK_a:
          chip8.SetKeyState(0x7, pressed);
          break;
        case SDLK_s:
          chip8.SetKeyState(0x8, pressed);
          break;
        case SDLK_d:
          chip8.SetKeyState(0x9, pressed);
          break;
        case SDLK_f:
          chip8.SetKeyState(0xE, pressed);
          break;

        case SDLK_z:
          chip8.SetKeyState(0xA, pressed);
          break;
        case SDLK_x:
          chip8.SetKeyState(0x0, pressed);
          break;
        case SDLK_c:
          chip8.SetKeyState(0xB, pressed);
          break;
        case SDLK_v:
          chip8.SetKeyState(0xF, pressed);
          break;
        }
      }
    }

    // ------------ timers 60 Hz ------------
    auto now = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(now - lastTimerUpdate).count();
    if (ms >= 16.67f)
    {
      if (chip8.delayTimer > 0)
        chip8.delayTimer--;
      if (chip8.soundTimer > 0)
        chip8.soundTimer--;
      lastTimerUpdate = now;
    }

    // ------------ CPU scheduler ------------
    if (unlimitedSpeed)
    {
      chip8.Cycle();
    }
    else
    {
      float micros = std::chrono::duration<float, std::micro>(now - lastCpu).count();
      float interval = 1'000'000.0f / cpuSpeedHz;
      if (micros >= interval)
      {
        chip8.Cycle();
        lastCpu = now;
      }
    }

    // ------------ render ------------
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    auto &vid = chip8.GetVideo();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < Chip8::VIDEO_HEIGHT; y++)
    {
      for (int x = 0; x < Chip8::VIDEO_WIDTH; x++)
      {
        if (vid[y * Chip8::VIDEO_WIDTH + x])
        {
          SDL_Rect rect{
              x * WINDOW_SCALE,
              y * WINDOW_SCALE,
              WINDOW_SCALE,
              WINDOW_SCALE};
          SDL_RenderFillRect(renderer, &rect);
        }
      }
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(1);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
