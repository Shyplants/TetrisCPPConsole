#include "Console.h"
#include "./utils/Logger.h"
#include <algorithm>
#include <vector>
#include <ShellScalingApi.h>
#include <cassert>

#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "User32.lib")

Console::Console()
	: m_hConsole{}
	, m_hConsoleWindow{}
	, m_ConsoleWindowRect{}
	, m_BytesWritten{ 0 }
	, m_pScreen{ nullptr }
{
	assert(EnableHighDpi() && "Failed to set enable high dpi");

	auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	bool bAdjustBuffer{ false };
	int minX{ GetSystemMetrics(SM_CXMIN) };
	int minY{ GetSystemMetrics(SM_CYMIN) };

	if (minX > SCREEN_WIDTH)
	{
		SCREEN_WIDTH = static_cast<SHORT>(minX);
		bAdjustBuffer = true;
	}

	if (minY > SCREEN_HEIGHT)
	{
		SCREEN_HEIGHT = static_cast<SHORT>(minY);
		bAdjustBuffer = true;
	}

	if (bAdjustBuffer)
	{
		BUFFER_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;
		HALF_WIDTH = SCREEN_WIDTH / 2;
		HALF_HEIGHT = SCREEN_HEIGHT / 2;
	}

	COORD consoleBuffer{ SCREEN_WIDTH, SCREEN_HEIGHT };
	if (!SetConsoleScreenBufferSize(hConsole, consoleBuffer))
	{
		auto error = GetLastError();
		TETRIS_ERROR("ERROR: " + std::to_string(error));
		assert(false && "Failed to set the console screen buffer size when creating the console!");
	}

	SMALL_RECT windowRect{ 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };
	if (!SetConsoleWindowInfo(hConsole, TRUE, &windowRect))
	{
		auto error = GetLastError();
		TETRIS_ERROR("ERROR: " + std::to_string(error));
		assert(false && "Failed to set the console window info when creating the console!");
	}


	Sleep(500);

	// Get a handle to the console window
	m_hConsoleWindow = GetConsoleWindow();

	if (!GetWindowRect(m_hConsoleWindow, &m_ConsoleWindowRect))
		assert(false && "Failed to get the Window Rect when creating the console!");

	HMONITOR mon = MonitorFromWindow(m_hConsoleWindow, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi{ sizeof(mi) };
	GetMonitorInfo(mon, &mi);  // mi.rcWork: 작업 표시줄 제외 영역 (픽셀, DPI 반영)

	// Center the window
	int posX = GetSystemMetrics(SM_CXSCREEN) / 2 - (m_ConsoleWindowRect.right - m_ConsoleWindowRect.left) / 2;
	int posY = GetSystemMetrics(SM_CYSCREEN) / 2 - (m_ConsoleWindowRect.bottom - m_ConsoleWindowRect.top) / 2;

	if (m_hConsoleWindow) {
		ShowWindow(m_hConsoleWindow, SW_RESTORE); // 최소화/숨김 방지
		SetWindowPos(m_hConsoleWindow, nullptr, posX, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	}

	// Initialize the screen buffer
	m_pScreen = std::make_unique<wchar_t[]>(BUFFER_SIZE);

	// Clear the screen buffer
	ClearBuffer();

	// Create the screen Buffer
	m_hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	if (!m_hConsole)
		assert(false && "Failed to create the console screen buffer!");

	// Set the buffer to be active!
	if (!SetConsoleActiveScreenBuffer(m_hConsole))
		assert(false && "Failed to set the active screen buffer");

	// Hide the cursor
	if (!ShowConsoleCursor(false))
		assert(false && "Failed to hide the console cursor!");
		

	SetConsoleTitleA("TetrisGame");
}

Console::~Console()
{
}

void Console::ClearBuffer()
{
	for (int i = 0; i < BUFFER_SIZE; ++i)
		m_pScreen[i] = L' ';
}

void Console::Write(int x, int y, const std::wstring& text, WORD color)
{
	static std::vector<wchar_t> invalidCharacters{ L' ', L'\n', L'\t', L'\r' };
	static std::vector<wchar_t> fullWideCharacters{ L'█' };

	auto is_any_invalid = [&](wchar_t character) {
		if (text.size() > 1)
			return false;

		if (text.empty())
			return true;

		return character == text[0];
	};

	auto is_any_fullWide = [&](wchar_t character) {
		return character == text[0];
	};

	// 전각 문자일 때
	if (std::find_if(fullWideCharacters.begin(), fullWideCharacters.end(), is_any_fullWide) != std::end(fullWideCharacters))
	{
		const int pos = y * SCREEN_WIDTH + x;

		assert(pos + text.size()*2 < BUFFER_SIZE);
		if (pos + text.size() * 2 >= BUFFER_SIZE)
		{
			TETRIS_ERROR("Trying to write to a position that is beyond the BUFFER SIZE!");
			return;
		}

		SetTextColor(text.size() * 2, x, y, m_hConsole, color);
		for (int i = 0; i < text.size(); ++i)
		{
			m_pScreen[pos + i * 2] = text[i];
		}
	}
	else // 반각 문자
	{
		const int pos = y * SCREEN_WIDTH + x;

		assert(pos + text.size() < BUFFER_SIZE);
		if (pos + text.size() >= BUFFER_SIZE)
		{
			TETRIS_ERROR("Trying to write to a position that is beyond the BUFFER SIZE!");
			return;
		}

		if (std::find_if(invalidCharacters.begin(), invalidCharacters.end(), is_any_invalid) == std::end(invalidCharacters))
			SetTextColor(text.size(), x, y, m_hConsole, color);

		swprintf(&m_pScreen[pos], BUFFER_SIZE - pos, L"%s", text.c_str());
		m_pScreen[pos + text.size()] = L' ';
	}

	//// 공백, 개행 등 단일 특수문자 제외시만 색 변경
	//if (std::find_if(invalidCharacters.begin(), invalidCharacters.end(), is_any_invalid) == std::end(invalidCharacters))
	//	SetTextColor(visualLen, x, y, m_hConsole, color);


	//// 전각 문자를 고려한 셀 단위 길이 계산
	//const int visualLen = ConsoleCellWidthOf(text);

	//int pos = y * SCREEN_WIDTH + x;

	//assert(pos + text.size() < BUFFER_SIZE);
	//if (pos + text.size() >= BUFFER_SIZE)
	//{
	//	TETRIS_ERROR("Trying to write to a position that is beyond the BUFFER SIZE!");
	//	return;
	//}

	//swprintf(&m_pScreen[pos], BUFFER_SIZE - pos, L"%s", text.c_str());
	//m_pScreen[pos + text.size()] = L' ';
}

void Console::Draw()
{
	WriteConsoleOutputCharacter(m_hConsole, m_pScreen.get(), BUFFER_SIZE, { 0, 0 }, &m_BytesWritten);
}

void Console::DrawHorz(int x, int y, size_t length, WORD color, const std::wstring& character)
{
	std::wstring sHorz = L"";
	for (int i = 0; i < length; ++i)
		sHorz += character;

	Write(x, y, sHorz, color);
}

void Console::DrawVert(int x, int y, size_t height, WORD color, const std::wstring& character)
{
	for (int i = 0; i < height; ++i)
		Write(x, y + i, character, color);
}

bool Console::ShowConsoleCursor(bool show)
{
	CONSOLE_CURSOR_INFO cursorInfo;
	if (!GetConsoleCursorInfo(m_hConsole, &cursorInfo))
	{
		TETRIS_ERROR("Failed to get the cursor info!");
		return false;
	}
	cursorInfo.bVisible = show;

	return SetConsoleCursorInfo(m_hConsole, &cursorInfo);
}

bool Console::EnableHighDpi()
{
	if (HMODULE user32 = ::GetModuleHandleW(L"user32.dll"))
	{
		using FnSetContext = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
		if (auto pSetContext = reinterpret_cast<FnSetContext>(
			::GetProcAddress(user32, "SetProcessDpiAwarenessContext")))
		{
			if (pSetContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
				return true;

			if (::GetLastError() == ERROR_ACCESS_DENIED)
				return true;

			if (pSetContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
				return true;
			if (::GetLastError() == ERROR_ACCESS_DENIED)
				return true;

			if (pSetContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE))
				return true;
			if (::GetLastError() == ERROR_ACCESS_DENIED)
				return true;
		}
	}

	return false;
}

bool Console::SetTextColor(int size, int x, int y, HANDLE handle, WORD color)
{
	COORD pos = { x, y };

	std::vector<WORD> write(size, color);

	DWORD written = 0;
	LPDWORD lpNumWritten = &written;

	if (!WriteConsoleOutputAttribute(handle, &write[0], size, pos, lpNumWritten))
	{
		TETRIS_ERROR("Unable to change text color!");
		return false;
	}

	return true;
}

//int Console::ConsoleCellWidth(wchar_t ch)
//{
//	WORD type = 0;
//
//	// CT_CTYPE3로 전각 여부 판정
//	if (GetStringTypeW(CT_CTYPE3, &ch, 1, &type))
//	{
//		if (type & C3_FULLWIDTH)
//			return 2;
//		if (type & C3_HALFWIDTH)
//			return 1;
//	}
//
//	// CT_CTYPE3로 판정이 나오지 않지만 전각 문자인것 들 ex) FULL BLOCK
//	static const std::unordered_set<wchar_t> USED_FULLWIDTH = {
//		0x2588
//	};
//
//	return USED_FULLWIDTH.count(ch) != 0 ? 2 : 1;
//}
//
//int Console::ConsoleCellWidthOf(const std::wstring& text)
//{
//	int w = 0;
//	for (wchar_t ch : text)
//		w += ConsoleCellWidth(ch);
//
//	return w;
//}
