# Compilation Fixes Applied

This document summarizes the changes made to make the Windows-specific C++ Minesweeper game compile on Linux.

## Original Issues
The original code was written for Windows using:
- EasyX graphics library (`graphics.h`, `easyx.h`)
- Windows-specific headers (`windows.h`, `conio.h`)
- Windows-specific functions and types

## Solutions Applied

### 1. Cross-Platform Compatibility Layer
Added conditional compilation blocks to handle Windows vs. Linux differences:
```cpp
#ifdef _WIN32
    // Windows-specific includes and code
#else
    // Linux stub implementations
#endif
```

### 2. Graphics Library Stubs
Created stub implementations for all EasyX graphics functions:
- `initgraph()`, `closegraph()`, `cleardevice()`
- `setfillcolor()`, `solidrectangle()`, `fillrectangle()`
- `settextcolor()`, `settextstyle()`, `outtextxy()`
- `putimage()`, `loadimage()`, `SetWorkingImage()`
- And many more...

### 3. Windows API Stubs
Implemented stub functions for Windows-specific APIs:
- `MessageBox()` - Windows message box function
- `GetKeyState()`, `GetAsyncKeyState()` - Keyboard state functions
- `Sleep()` - Sleep function (mapped to `usleep()` on Linux)
- Mouse and keyboard event handling functions

### 4. Type Definitions
Added cross-platform type definitions:
- `COLORREF` - Color reference type
- `IMAGE` - Image structure with member functions
- `ExMessage`, `MOUSEMSG` - Message structures
- `RECT` - Rectangle structure

### 5. Macro Definitions
Added missing macro definitions:
- Color constants (`WHITE`, `BLACK`, `DARKGRAY`, `RED`)
- Message constants (`WM_LBUTTONDOWN`, `WM_RBUTTONDOWN`)
- Virtual key codes (`VK_UP`, `VK_DOWN`, `VK_LEFT`, `VK_RIGHT`, etc.)
- MessageBox constants (`MB_OK`, `MB_OKCANCEL`, `IDOK`)

### 6. File I/O Compatibility
Fixed wide character file stream issues:
- Replaced `std::wifstream`/`std::wofstream` with conditional compilation
- Added helper functions to convert between `std::wstring` and `std::string`
- Used regular file streams on Linux with string conversion

### 7. String Function Compatibility
- Replaced `swprintf_s` with cross-platform macro using `swprintf`
- Replaced `_wtoi` with `wcstol` wrapper
- Added `_T` macro for text literals

### 8. Missing Function Implementations
Added stub implementations for declared but undefined functions:
- `showGameInstructions()`
- `showGameControls()`

## Result
The code now compiles successfully on Linux using g++ and produces a working executable. While the graphics functions are stubbed out (so no visual output), the core game logic and structure remain intact and functional.

## Usage
```bash
g++ -o minesweeper main.cpp
./minesweeper
```

The program will run but won't display graphics since the graphics functions are stubbed. To make it fully functional, you would need to:
1. Implement the graphics functions using a cross-platform library like SDL2 or SFML
2. Or create a console-based version of the game
3. Or use the original code on a Windows system with EasyX installed