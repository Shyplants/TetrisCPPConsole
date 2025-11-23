#pragma once

#include "IState.h"
#include "../Selector.h"
#include "../utils/Types.h"
#include "../common/TetrisTypes.h"

class Console;
class Keyboard;
class SoundManager;
class StateMachine;

class Board;
class BagRandom;
class Score;
class Timer;
class ConsoleRenderer;

class Tetromino;

class SinglePlayState final : public IState
{
public:
	SinglePlayState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine);
	~SinglePlayState() override;

	void OnEnter() override;
	void OnExit() override;
	void Update() override;
	void Draw() override;
	void ProcessInputs() override;

	bool Exit() override;

private:

	// 게임 로직
	bool TrySpawnMino();
	bool TryMove(int dx, int dy);
	bool TryRotateCW();
	bool TryRotateCCW();
	bool TryHold();
	void HardDrop();
	void LockAndProceed();
	int GravityIntervalMS() const;

	void UpdatePreviewMinos();
	void UpdateGhostMino();

	void OnComboAchieved(int comboCount);
	void OnGameOver();

	// 사용한 자원 초기화
	void Shutdown();

private:
	Console& m_Console;
	Keyboard& m_Keyboard;
	SoundManager& m_SoundManager;
	StateMachine& m_StateMachine;

	std::unique_ptr<Board> m_Board;
	std::unique_ptr<BagRandom> m_Bag;
	std::unique_ptr<Score> m_Score;

	std::unique_ptr<Timer> m_GravityTimer;
	std::unique_ptr<Timer> m_PlayTimer;
	std::unique_ptr<Timer> m_ComboTimer;
	std::unique_ptr<Timer> m_SoftDropTimer;
	std::unique_ptr<Timer> m_GameOverTimer;

	// 화면/보드 배치 정보
	int m_ScreenWidth, m_ScreenHeight, m_CenterScreenW, m_CenterScreenH;
	int m_BoardLeft, m_BoardTop;

	std::unique_ptr<ConsoleRenderer> m_Renderer;

	bool m_bGameReady{ false };
	bool m_bGameOver{ false };
	bool m_bWaitingGameOverTransition{ false };

	// 현재 미노
	std::unique_ptr<Tetromino> m_CurMino{ nullptr };

	// 미리보기 미노
	std::array<Tetris::TetrominoType, Tetris::MINO_PREVIEW_COUNT> m_PreviewMinos{};

	// 고스트 미노
	std::unique_ptr<Tetromino> m_GhostMino{ nullptr };

	// 홀드 미노 타입
	Tetris::TetrominoType m_holdMinoType{ Tetris::TetrominoType::None };
	bool m_bHasHeldThisTurn{ false };

	// 누적 피스 갯수
	int m_totalPieces{ 0 };

	// 콤보 출력을 위한 변수
	int m_LastCombo{ 0 };
	bool m_bShowCombo{ false };
};