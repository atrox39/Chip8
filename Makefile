build:
	g++ -std=c++17 src/main.cpp src/chip8.cpp -lcomdlg32 -luser32 -lgdi32 -lmingw32 -lSDL2main -lSDL2 -o chip8

