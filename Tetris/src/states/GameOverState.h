#pragma once

#include "IState.h"
#include "../Selector.h"
#include "../utils/Types.h"

class Console;
class Keyboard;
class SoundManager;
class StateMachine;

class GameOverState final : public IState
{
public:
	GameOverState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine, bool m_bIsVictory = true);
	~GameOverState() override;

	void OnEnter() override;
	void OnExit() override;
	void Update() override;
	void Draw() override;
	void ProcessInputs() override;

	bool Exit() override;

private:
	void OnMenuSelect(int index, std::vector<std::wstring> data);
	

private:
	Console& m_Console;
	Keyboard& m_Keyboard;
	SoundManager& m_SoundManager;
	StateMachine& m_StateMachine;

	Selector<> m_MenuSelector;

	// 화면 배치 정보
	int m_ScreenWidth, m_ScreenHeight, m_CenterScreenW, m_CenterScreenH;

	bool m_bIsVictory;


};