#include "GameOverState.h"
#include "../Console.h"
#include "../inputs/Keyboard.h"
#include "../audio/SoundManager.h"
#include "StateMachine.h"
#include "../utils/Logger.h"
#include <cassert>

using namespace std::placeholders;

GameOverState::GameOverState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine, bool m_bIsVictory)
	: m_Console{ console }
	, m_Keyboard{ keyboard }
	, m_SoundManager{ soundManager }
	, m_StateMachine{ stateMachine }
	, m_bIsVictory{ m_bIsVictory }
	, m_MenuSelector{ console, keyboard, {L"Title", L"Exit"}, SelectorParams{console.GetHalfWidth(), console.GetHalfHeight()} }
	, m_ScreenWidth{ console.GetScreenWidth() }
	, m_ScreenHeight{ console.GetScreenHeight() }
	, m_CenterScreenW{ console.GetHalfWidth() }
	, m_CenterScreenH{ console.GetHalfHeight() }
{
	m_MenuSelector.SetSelectionFunc(std::bind(&GameOverState::OnMenuSelect, this, _1, _2));
}

GameOverState::~GameOverState()
{
}

void GameOverState::OnEnter()
{
	m_Console.ClearBuffer();
}

void GameOverState::OnExit()
{
	m_Console.ClearBuffer();
}

void GameOverState::Update()
{

	
}

void GameOverState::Draw()
{
	m_Console.ClearBuffer();

	const int panel_x = m_CenterScreenW - 2;
	const int panel_y = m_CenterScreenH - 5;

	int current_y = panel_y;
	

	// 게임 종료 문구 출력
	
	if (m_bIsVictory)
	{
		m_Console.Write(panel_x, current_y, L"You Win!", GREEN);
	}
	else
	{
		m_Console.Write(panel_x, current_y, L"You Lose..", RED);
	}

	
	current_y += 3;

	m_MenuSelector.Draw();
	
}

void GameOverState::ProcessInputs()
{
	m_MenuSelector.ProcessInputs();
}

bool GameOverState::Exit()
{
	return false;
}

void GameOverState::OnMenuSelect(int index, std::vector<std::wstring> data)
{
	switch (index)
	{
	case 0: // Title
		m_StateMachine.RequestPopDepth(1);
		return;

	case 2:
		// Game Exit
		break;
	}
}