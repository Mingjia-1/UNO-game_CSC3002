#include "unoui.h"
#include <iostream>
#include <conio.h>
#include <windows.h>

int main() {
    // Enable ANSI escape codes for colors
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
    
    UnoUI game;
    
    while (game.isGameRunning()) {
        game.drawGameScreen();
        game.handleInput();
        
        // Handle CPU turns automatically
        if (game.isCPUTurn()) {
            Sleep(1500); // Delay to make CPU turns visible
        }
        
        Sleep(100);
    }
    
    std::cout << "Game ended. Press any key to exit...";
    _getch();
    return 0;
}