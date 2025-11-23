#pragma once

#include "IState.h"
#include "../Selector.h"

class Console;
class Keyboard;
class SoundManager;
class StateMachine;

class TitleState final : public IState
{
public:
	TitleState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine);
	~TitleState() override;

	void OnEnter() override;
	void OnExit() override;
	void Update() override;
	void Draw() override;
	void ProcessInputs() override;

	bool Exit() override;

private:
	void DrawPanels();

	void OnMenuSelect(int index, std::vector<std::wstring> data);

private:
	static constexpr int PANEL_BARS = 90;
	static constexpr int TITLE_SIZE = 46;

	Console& m_Console;
	Keyboard& m_Keyboard;
	SoundManager& m_SoundManager;
	StateMachine& m_StateMachine;

	Selector<> m_MenuSelector;

	bool m_bExitGame{ false }, m_bInSelect{ true };
	int m_ScreenWidth, m_ScreenHeight, m_CenterScreenW, m_PanelBarX;

};