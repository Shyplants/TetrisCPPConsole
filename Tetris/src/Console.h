#pragma once
#define WIN32_LEAN_AND_MEAN

#include "./utils/Colors.h"
#include <Windows.h>
#include <memory>
#include <string>

class Console
{
public:
	Console();
	~Console();

	const SHORT GetScreenWidth() const { return SCREEN_WIDTH; }
	const SHORT GetScreenHeight() const { return SCREEN_HEIGHT; }
	const SHORT GetHalfWidth() const { return HALF_WIDTH; }
	const SHORT GetHalfHeight() const { return HALF_HEIGHT; }

	void ClearBuffer();
	void Write(int x, int y, const std::wstring& text, WORD color = WHITE);
	void Draw();
	void DrawHorz(int x, int y, size_t length, WORD color = WHITE, const std::wstring& character = L"█");
	void DrawVert(int x, int y, size_t height, WORD color = WHITE, const std::wstring& character = L"█");
	bool ShowConsoleCursor(bool show);
	
	bool EnableHighDpi();

private:
	bool SetTextColor(int size, int x, int y, HANDLE handle, WORD color);

private:
	SHORT SCREEN_WIDTH = 198;
	SHORT SCREEN_HEIGHT = 56;
	SHORT BUFFER_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;
	SHORT HALF_WIDTH = SCREEN_WIDTH / 2;
	SHORT HALF_HEIGHT = SCREEN_HEIGHT / 2;

	HANDLE m_hConsole;
	HWND m_hConsoleWindow;
	RECT m_ConsoleWindowRect;

	DWORD m_BytesWritten;
	std::unique_ptr<wchar_t[]> m_pScreen;
};