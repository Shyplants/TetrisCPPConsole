#pragma once

#include "IState.h"
#include <memory>
#include "../common/PacketProtocol.h"
#include "../common/TetrisTypes.h"

class Console;
class Keyboard;
class SoundManager;
class StateMachine;

class Board;
class BagRandom;
class Score;
class Timer;
class Tetromino;

class ConsoleRenderer;

class TetrisClient;

class MultiPlayState final : public IState
{
public:
    MultiPlayState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine, uint64_t bagSeed);
	~MultiPlayState() override;

	void OnEnter() override;
	void OnExit() override;
	void Update() override;
	void Draw() override;
	void ProcessInputs() override;

	bool Exit() override;

public:
	void SetClient(std::unique_ptr<TetrisClient> client);

private:
    void DrawBoards();
    void DrawInfoPanels();

	// 게임 로직
	bool TrySpawnMino();
	bool TryMove(int dx, int dy);
	bool TryRotateCW();
	bool TryRotateCCW();
	bool TryHold();
	void HardDrop();
	void LockAndProceed();
	int GravityIntervalMS() const;

	void UpdateMyGhostMino();
	void UpdateEnemyGhostMino();

	void UpdateMyPreviewMinos();

	void OnComboAchieved(int comboCount);
	void OnClientGameOver();

	// 네트워크 로직
	void SendCurMinoState();
	void SendHoldMinoState();
	void SendPreviewMinoState();
	void SendBoardState();
	void SendClientGameOver();

	void OnServerGameOver();

    void Shutdown();

private:
	void HandlePackets();

private:
	Console& m_Console;
	Keyboard& m_Keyboard;
	SoundManager& m_SoundManager;
	StateMachine& m_StateMachine;

	std::unique_ptr<TetrisClient> m_Client{ nullptr };

	std::unique_ptr<Board> m_MyBoard;
	std::unique_ptr<Board> m_EnemyBoard;

	std::unique_ptr<BagRandom> m_MyBag;
	std::unique_ptr<BagRandom> m_EnemyBag;

	std::unique_ptr<Score> m_Score;
	std::unique_ptr<Timer> m_GravityTimer;
	std::unique_ptr<Timer> m_PlayTimer;
	std::unique_ptr<Timer> m_ComboTimer;
	std::unique_ptr<Timer> m_SoftDropTimer;
	std::unique_ptr<Timer> m_GameOverTimer;

	// 화면/보드 배치 정보
	int m_ScreenWidth, m_ScreenHeight, m_CenterScreenW, m_CenterScreenH;
	int m_BoardLeft, m_BoardTop;

	std::unique_ptr<ConsoleRenderer> m_MyRenderer;
	std::unique_ptr<ConsoleRenderer> m_EnemyRenderer;

	bool m_bClientReady{ false };
	bool m_bRemoteReady[2]{ false, };
	bool m_bClientGameOver{ false };
	bool m_bServerGameOver{ false };
	bool m_bWaitingGameOverTransition{ false };

	// 현재 미노
	std::unique_ptr<Tetromino> m_MyCurMino{ nullptr };
	std::unique_ptr<Tetromino> m_EnemyCurMino{ nullptr };

	// 미리보기 미노
	std::array<Tetris::TetrominoType, Tetris::MINO_PREVIEW_COUNT> m_MyPreviewMinos{};
	std::array<Tetris::TetrominoType, Tetris::MINO_PREVIEW_COUNT> m_EnemyPreviewMinos{};

	// 고스트 미노
	std::unique_ptr<Tetromino> m_MyGhostMino{ nullptr };
	std::unique_ptr<Tetromino> m_EnemyGhostMino{ nullptr };

	// 홀드 미노 타입
	Tetris::TetrominoType m_MyHoldMinoType{ Tetris::TetrominoType::None };
	Tetris::TetrominoType m_EnemyHoldMinoType{ Tetris::TetrominoType::None };
	bool m_bHasHeldThisTurn{ false };

	// 누적 피스 갯수
	int m_totalPieces{ 0 };

	// 콤보 출력을 위한 변수
	int m_LastCombo{ 0 };
	bool m_bShowCombo{ false };

	// 멀티 플레이 결과
	bool m_bIsVictory{ false };
};