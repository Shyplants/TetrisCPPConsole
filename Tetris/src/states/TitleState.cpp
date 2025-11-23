#include "TitleState.h"
#include "../Console.h"
#include "../inputs/Keyboard.h"
#include "../audio/SoundManager.h"
#include "StateMachine.h"
#include "SinglePlayState.h"
#include "RoomJoinState.h"
#include "../utils/Logger.h"

// Test
#include "MultiPlayState.h"

using namespace std::placeholders;

TitleState::TitleState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine)
	: m_Console{ console }
	, m_Keyboard{ keyboard }
	, m_SoundManager{ soundManager }
	, m_StateMachine{ stateMachine }
	, m_MenuSelector{ console, keyboard, {L"Single Play", L"Multi Play", L"Exit"}, SelectorParams{console.GetHalfWidth(), console.GetHalfHeight()}}
	, m_ScreenWidth{ console.GetScreenWidth() }
	, m_ScreenHeight{ console.GetScreenHeight() }
	, m_CenterScreenW{ console.GetHalfWidth() }
	, m_PanelBarX{ m_CenterScreenW - (PANEL_BARS / 2) }
{
	m_MenuSelector.SetSelectionFunc(std::bind(&TitleState::OnMenuSelect, this, _1, _2));
}

TitleState::~TitleState()
{
}

void TitleState::OnEnter()
{
	m_SoundManager.PlayBGM("title_bgm");

	m_Console.ClearBuffer();
}

void TitleState::OnExit()
{
	m_Console.ClearBuffer();
}

void TitleState::Update()
{
}

void TitleState::Draw()
{
	DrawPanels();

	m_MenuSelector.Draw();
}

void TitleState::ProcessInputs()
{
	m_MenuSelector.ProcessInputs();
}

bool TitleState::Exit()
{
	return false;
}

void TitleState::DrawPanels()
{
	// Draw Title
	int title_x_pos = m_CenterScreenW - (TITLE_SIZE / 2);
	m_Console.Write(title_x_pos, 2, L"  _______ ______ _______ _____  _____  _____", GREEN);
	m_Console.Write(title_x_pos, 3, L" |__   __|  ____|__   __|  __ \\|_   _|/ ____|", GREEN);
	m_Console.Write(title_x_pos, 4, L"    | |  | |__     | |  | |__) | | | | (___", GREEN);
	m_Console.Write(title_x_pos, 5, L"    | |  |  __|    | |  |  _  /  | |  \\___ \\", GREEN);
	m_Console.Write(title_x_pos, 6, L"    | |  | |____   | |  | | \\ \\ _| |_ ____) |", GREEN);
	m_Console.Write(title_x_pos, 7, L"    |_|  |______|  |_|  |_|  \\_\\_____|_____/", GREEN);

}

void TitleState::OnMenuSelect(int index, std::vector<std::wstring> data)
{
	switch (index)
	{
	case 0: // Single Play
		m_StateMachine.PushState(std::make_unique<SinglePlayState>(m_Console, m_Keyboard, m_SoundManager, m_StateMachine));
		break;	

	case 1: // Multi Play (1 vs 1)
		m_StateMachine.PushState(std::make_unique<RoomJoinState>(m_Console, m_Keyboard, m_SoundManager, m_StateMachine));
		break;
	case 2:
		// Game Exit
		break;
	}
}
