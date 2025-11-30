# CHIP-8 Emulator

Un emulador de CHIP-8 escrito en C++ con interfaz gráfica usando SDL2 y menús nativos de Windows.

## Descripción

CHIP-8 es un lenguaje de programación interpretado desarrollado en la década de 1970, diseñado originalmente para facilitar el desarrollo de videojuegos en microcomputadoras. Este emulador recrea el sistema CHIP-8 completo, permitiendo ejecutar ROMs clásicas con una interfaz moderna.

## Características

- ✅ Emulación completa de CHIP-8 (35 instrucciones)
- 🎮 Soporte de teclado hexadecimal (16 teclas)
- 🖥️ Pantalla de 64x32 píxeles escalada
- ⚡ Velocidad de CPU ajustable (200 Hz - 1500 Hz + modo ilimitado)
- 📂 Cargador de ROMs mediante diálogo de archivo nativo
- 🎵 Timers de delay y sound implementados
- 🪟 Menús nativos de Windows integrados

## Requisitos

- **Compilador:** GCC con soporte C++17 o superior
- **Librerías:**
  - SDL2 (Simple DirectMedia Layer 2)
  - Windows API (commdlg32, user32, gdi32)

## Compilación

### Windows con MinGW

```bash
make build
```

Esto generará el ejecutable `chip8.exe`.

### Comando de compilación manual

```bash
g++ -std=c++17 src/main.cpp src/chip8.cpp -lcomdlg32 -luser32 -lgdi32 -lmingw32 -lSDL2main -lSDL2 -o chip8
```

## Uso

### Ejecutar con una ROM

```bash
chip8.exe <ruta_al_archivo.ch8>
```

Por ejemplo:
```bash
chip8.exe roms/games/Pong.ch8
```

### Controles

El teclado CHIP-8 original usa 16 teclas (0-F) que están mapeadas de la siguiente forma:

```
Teclado CHIP-8:        Teclado PC:
┌───┬───┬───┬───┐     ┌───┬───┬───┬───┐
│ 1 │ 2 │ 3 │ C │     │ 1 │ 2 │ 3 │ 4 │
├───┼───┼───┼───┤     ├───┼───┼───┼───┤
│ 4 │ 5 │ 6 │ D │     │ Q │ W │ E │ R │
├───┼───┼───┼───┤     ├───┼───┼───┼───┤
│ 7 │ 8 │ 9 │ E │     │ A │ S │ D │ F │
├───┼───┼───┼───┤     ├───┼───┼───┼───┤
│ A │ 0 │ B │ F │     │ Z │ X │ C │ V │
└───┴───┴───┴───┘     └───┴───┴───┴───┘
```

### Menús

#### File
- **Open ROM**: Abre un diálogo para cargar una nueva ROM
- **Exit**: Cierra el emulador

#### Speed
Permite ajustar la velocidad de ejecución de la CPU:
- 200 Hz
- 300 Hz
- **500 Hz** (por defecto)
- 700 Hz
- 1000 Hz
- 1500 Hz
- Max (Unlimited) - Sin límite de velocidad

## Estructura del Proyecto

```
chip8/
├── src/
│   ├── main.cpp      # Punto de entrada y manejo de SDL/Windows
│   ├── chip8.cpp     # Implementación del emulador
│   └── chip8.hpp     # Definición de la clase Chip8
├── roms/
│   ├── games/        # ROMs de juegos
│   ├── demos/        # ROMs de demostración
│   ├── hires/        # ROMs de alta resolución
│   └── programs/     # Programas utilitarios
├── Makefile          # Script de compilación
└── README.md         # Este archivo
```

## Arquitectura del Emulador

### Componentes Principales

- **Memoria**: 4KB (4096 bytes)
- **Registros**: 16 registros de 8 bits (V0-VF)
- **Registro I**: 16 bits para direcciones de memoria
- **Program Counter (PC)**: Apunta a la instrucción actual
- **Stack**: 16 niveles para guardar direcciones de retorno
- **Timers**: Delay timer y Sound timer (60 Hz)
- **Display**: 64x32 píxeles monocromáticos

### Ciclo de Ejecución

1. **Fetch**: Leer opcode de la memoria
2. **Decode**: Interpretar el opcode
3. **Execute**: Ejecutar la instrucción
4. **Update**: Actualizar timers y pantalla

## Recursos

- [Especificación CHIP-8](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Guía de Cowgod sobre CHIP-8](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Awesome CHIP-8](https://github.com/tobiasvl/awesome-chip-8)

## Licencia

Este proyecto es de código abierto. Siéntete libre de usar, modificar y distribuir según necesites.

## Autor

**atrox39**

## Contribuciones

Las contribuciones son bienvenidas. Si encuentras algún bug o quieres añadir nuevas características, no dudes en abrir un issue o pull request.
