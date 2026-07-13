#include <iostream>
#include <windows.h>

#include "Renderer.h"

Renderer::Renderer() {
	clear();
}

void Renderer::setColor(int color) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}

void Renderer::drawChar(const int x, const int y, const int color, const wchar_t c) {
    if (x >= 0 && x < w && y >= 0 && y < h) {
        char_buffer[y][x] = c;
        color_buffer[y][x] = color;
    }
}

void Renderer::drawString(const int x, const int y, const int color, const std::wstring str) {
    for (int i = 0; i < str.length(); i++) {
        if (x + i >= 0 && x + i < w && y >= 0 && y < h) {
            char_buffer[y][x + i] = str[i];
            color_buffer[y][x + i] = color;
        }
    }
}

void Renderer::clear() {
    for (auto& row : char_buffer) {
        row.fill(' ');
    }
    for (auto& row : color_buffer) {
        row.fill(7);
    }
    
    for (auto& row : prev_char_buffer) {
        row.fill(L'\0');
    }
    for (auto& row : prev_color_buffer) {
        row.fill(-1);
    }
}

void Renderer::render() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    int current_color = 7;
    SetConsoleTextAttribute(hOut, current_color);

    for (short y = 0; y < h; y++) {
        for (short x = 0; x < w; x++) {           
            if (char_buffer[y][x] != prev_char_buffer[y][x] || color_buffer[y][x] != prev_color_buffer[y][x]) {

                COORD coord = { x, y };
                SetConsoleCursorPosition(hOut, coord);

                if (color_buffer[y][x] != current_color) {
                    current_color = color_buffer[y][x];
                    SetConsoleTextAttribute(hOut, current_color);
                }

                std::wcout << char_buffer[y][x];

                prev_char_buffer[y][x] = char_buffer[y][x];
                prev_color_buffer[y][x] = color_buffer[y][x];
            }
        }
    }
}

