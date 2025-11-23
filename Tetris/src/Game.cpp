#include "Game.h"
#include "Console.h"
#include "./inputs/Keyboard.h"
#include "./audio/SoundManager.h"
#include "./states/StateMachine.h"

#include "./utils/Logger.h"
#include "./states/TitleState.h"
#include <iostream>

Game::Game()
{
}

Game::~Game()
{

}

void Game::Run()
{
	if (!Init())
		m_bIsRunning = false;

	while (m_bIsRunning)
	{
		ProcessEvents();
		ProcessInputs();
		Update();
		Draw();
	}

	std::cout << "Game Ended\n";
}

bool Game::Init()
{
	m_pConsole = std::make_unique<Console>();
	if (!m_pConsole)
	{
		TETRIS_ERROR("Failed to create Console!");
		return false;
	}

	m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
	
	m_pKeyboard = std::make_unique<Keyboard>();
	if (!m_pKeyboard)
	{
		TETRIS_ERROR("Failed to create Keyboard!");
		return false;
	}

	m_pSoundManager = std::make_unique<SoundManager>();
	if (!m_pSoundManager)
	{
		TETRIS_ERROR("Failed to create SoundManager!");
		return false;
	}
	if (!m_pSoundManager->Init())
	{
		TETRIS_ERROR("Failed to initialize SoundManager!");
		return false;
	}

	// 초기 사운드 조절
	{
		m_pSoundManager->SetMasterVolume(0.8f);
		m_pSoundManager->SetBGMVolume(0.5f);
		m_pSoundManager->SetSEVolume(1.0f);
	}

	// BGM 로드
	{
		m_pSoundManager->LoadBGM("title_bgm", "assets/audio/title_bgm.wav", true);
		m_pSoundManager->LoadBGM("play_bgm", "assets/audio/play_bgm.mp3", true);
	}

	// 인게임 사운드 로드
	{
		m_pSoundManager->LoadSE("move", "assets/audio/Tetrio_retro_pack/move.wav");
		m_pSoundManager->LoadSE("hold", "assets/audio/Tetrio_retro_pack/hold.wav");
		m_pSoundManager->LoadSE("softdrop", "assets/audio/Tetrio_retro_pack/softdrop.wav");
		m_pSoundManager->LoadSE("harddrop", "assets/audio/Tetrio_retro_pack/harddrop.wav");
		m_pSoundManager->LoadSE("floor", "assets/audio/Tetrio_retro_pack/floor.wav");
		m_pSoundManager->LoadSE("failure", "assets/audio/Tetrio_retro_pack/failure.wav");

		// 콤보 사운드
		for (int i = 1; i <= 16; ++i)
		{
			std::string combo_string = "combo_" + std::to_string(i);
			std::string combo_filepath = "assets/audio/Tetrio_retro_pack/" + combo_string + ".wav";
			m_pSoundManager->LoadSE(combo_string, combo_filepath);
		}
	}

	m_pStateMachine = std::make_unique<StateMachine>();
	if (!m_pStateMachine)
	{
		TETRIS_ERROR("Failed to create StateMachine!");
		return false;
	}

	m_pStateMachine->PushState(std::make_unique<TitleState>(*m_pConsole, *m_pKeyboard, *m_pSoundManager, *m_pStateMachine));

	return true;
}

void Game::ProcessEvents()
{
	// Get the number of Console Inputs
	if (!GetNumberOfConsoleInputEvents(m_hConsoleIn, &m_NumInputEvents))
	{
		DWORD error = GetLastError();
		TETRIS_ERROR("Failed to get number of console input events!" + error);
		return;
	}

	if (m_NumInputEvents <= 0)
		return;

	if (!PeekConsoleInput(m_hConsoleIn, m_InRecBuf, INPUT_RECORD_BUF_SIZE, &m_NumInputEvents))
	{
		DWORD error = GetLastError();
		TETRIS_ERROR("Failed to Peek Events!" + error);
		return;
	}

	for (int i = 0; i < m_NumInputEvents; ++i)
	{
		switch (m_InRecBuf[i].EventType)
		{
		case KEY_EVENT:
			KeyEventProcess(m_InRecBuf[i].Event.KeyEvent);
			break;

		default:
			break;
		}
	}

	// Clear or Flush Other inputs
	FlushConsoleInputBuffer(m_hConsoleIn);
}

void Game::ProcessInputs()
{
	if (m_pKeyboard->IsKeyJustPressed(KEY_ESCAPE))
		m_bIsRunning = false;

	if (m_pStateMachine->Empty())
	{
		TETRIS_ERROR("NO STATE IN STATE MACHINE TO PROCESS INPUTS!");
		m_bIsRunning = false;
		return;
	}

	m_pStateMachine->GetCurrentState()->ProcessInputs();
}

void Game::Update()
{
	if (m_pStateMachine->Empty())
	{
		TETRIS_ERROR("NO STATE IN STATE MACHINE TO UPDATE!");
		m_bIsRunning = false;
		return;
	}

	m_pStateMachine->Update();
	m_pStateMachine->GetCurrentState()->Update();
	m_pKeyboard->Update();
}

void Game::Draw()
{
	if (m_pStateMachine->Empty())
	{
		TETRIS_ERROR("NO STATE IN STATE MACHINE TO DRAW!");
		m_bIsRunning = false;
		return;
	}

	m_pStateMachine->GetCurrentState()->Draw();
	m_pConsole->Draw();
}

void Game::KeyEventProcess(KEY_EVENT_RECORD keyEvent)
{
	if (keyEvent.bKeyDown)
		m_pKeyboard->OnKeyDown(keyEvent.wVirtualKeyCode);
	else
		m_pKeyboard->OnKeyUp(keyEvent.wVirtualKeyCode);
}