#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <memory>
#include <thread>

class Console;
class Keyboard;
class SoundManager;
class StateMachine;

class Game
{
public:
	Game();
	~Game();

	void Run();

private:
	bool Init();

	void ProcessEvents();
	void ProcessInputs();
	void Update();
	void Draw();

	void KeyEventProcess(KEY_EVENT_RECORD keyEvent);

private:
	static constexpr DWORD INPUT_RECORD_BUF_SIZE = 128;

	bool m_bIsRunning{ true };

	std::unique_ptr<Console> m_pConsole{ nullptr };
	std::unique_ptr<Keyboard> m_pKeyboard{ nullptr };
	std::unique_ptr<SoundManager> m_pSoundManager{ nullptr };
	std::unique_ptr<StateMachine> m_pStateMachine{ nullptr };

	
	DWORD m_NumInputEvents{ 0 };
	INPUT_RECORD m_InRecBuf[INPUT_RECORD_BUF_SIZE]{};
	HANDLE m_hConsoleIn{ nullptr };

};