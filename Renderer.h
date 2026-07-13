#pragma once

#include <string>
#include <array>

class Renderer {
private:
	inline static const int w = 80, h = 25;

	inline static std::array<std::array<wchar_t, w>, h> char_buffer;
	inline static std::array<std::array<int, w>, h> color_buffer;

	// previous frame buffers
	inline static std::array<std::array<wchar_t, w>, h> prev_char_buffer;
	inline static std::array<std::array<int, w>, h> prev_color_buffer;
public:
	Renderer();

	static void setColor(int color);

	static void drawChar(const int x, const int y, const int color, const wchar_t c);
	static void drawString(const int x, const int y, const int color, const std::wstring str);
	static void clear();
	static void render();
};