#include "SinglePlayState.h"
#include "../Console.h"
#include "../inputs/Keyboard.h"
#include "../audio/SoundManager.h"
#include "StateMachine.h"
#include "../utils/Logger.h"
#include <cassert>

#include "../Board.h"
#include "../BagRandom.h"
#include "../Score.h"
#include "../utils/Timer.h"
#include "../Tetromino.h"
#include "../GameConfig.h"

#include "../ConsoleRenderer.h"
#include "../common/TetrisTypes.h"
#include "GameOverState.h"

SinglePlayState::SinglePlayState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine)
	: m_Console{ console }
	, m_Keyboard{ keyboard }
	, m_SoundManager{ soundManager }
	, m_StateMachine{ stateMachine }
	, m_Board{ std::make_unique<Board>() }
	, m_Bag{ std::make_unique<BagRandom>() }
	, m_Score{ std::make_unique<Score>() }
	, m_GravityTimer{ std::make_unique<Timer>() }
	, m_PlayTimer{ std::make_unique<Timer>() }
	, m_ComboTimer{ std::make_unique<Timer>() }
	, m_SoftDropTimer{ std::make_unique<Timer>() }
	, m_GameOverTimer{ std::make_unique<Timer>() }
	, m_ScreenWidth{ console.GetScreenWidth() }
	, m_ScreenHeight{ console.GetScreenHeight() }
	, m_CenterScreenW{ console.GetHalfWidth() }
	, m_CenterScreenH{ console.GetHalfHeight() }
	, m_BoardLeft{ m_CenterScreenW - BOARD_WIDTH }
	, m_BoardTop{ m_CenterScreenH - (BOARD_HEIGHT / 2) }
	, m_Renderer{ std::make_unique<ConsoleRenderer>(console, m_BoardLeft, m_BoardTop) }
{
}

SinglePlayState::~SinglePlayState()
{
}

void SinglePlayState::OnEnter()
{
	m_Console.ClearBuffer();

	m_SoundManager.PlayBGM("play_bgm");

	m_GravityTimer->Start();
	m_PlayTimer->Start();
	m_ComboTimer->Start();
	m_SoftDropTimer->Start();

	TrySpawnMino();
}

void SinglePlayState::OnExit()
{
	Shutdown();
}

void SinglePlayState::Update()
{
	if (m_bWaitingGameOverTransition)
	{
		if (m_GameOverTimer->ElapsedMS() >= 3000) // 3초
		{
			m_bWaitingGameOverTransition = false;

			// GameOverState로 전이
			m_StateMachine.PushState(std::make_unique<GameOverState>(m_Console, m_Keyboard, m_SoundManager, m_StateMachine));
		}
	}

	if (m_bGameOver)
	{
		return;
	}

	// 중력 낙하
	if (m_GravityTimer->ElapsedMS() >= GravityIntervalMS())
	{
		if (!TryMove(0, +1))
		{
			LockAndProceed();
		}
		else
			m_GravityTimer->Restart();
	}

	UpdatePreviewMinos();

	// 고스트 미노 업데이트
	UpdateGhostMino();

	if (!m_bGameReady)
		m_bGameReady = true;

	
}

void SinglePlayState::Draw()
{
	if (!m_bGameReady)
		return;

	m_Console.ClearBuffer();

	m_Renderer->DrawBoard(*m_Board, m_CurMino.get(), m_GhostMino.get());
	m_Renderer->DrawPreviewPanel(m_PreviewMinos);
	m_Renderer->DrawHoldPanel(m_holdMinoType);
	m_Renderer->DrawInfoPanel(*m_Score, *m_PlayTimer, m_totalPieces, m_LastCombo, m_bShowCombo, *m_ComboTimer);
}

void SinglePlayState::ProcessInputs()
{
	if (m_bGameOver)
	{
		return;
	}

	if (m_Keyboard.IsKeyJustPressed(KEY_UP))
	{
		if (TryRotateCW())
		{

		}
	}

	if (m_Keyboard.IsKeyJustPressed(KEY_Z))
	{
		if (TryRotateCCW())
		{
		}
	}

	else if (m_Keyboard.IsKeyJustPressed(KEY_DOWN) ||
		(m_Keyboard.IsKeyHeld(KEY_DOWN) && m_SoftDropTimer->ElapsedMS() >= GameConfig::SoftDropIntervalMS))
	{
		if (TryMove(0, +1))
		{
			m_SoundManager.PlaySE_Force("move");
			m_Score->AddSoftDrop(1);

			m_SoftDropTimer->Restart();
			m_GravityTimer->Restart();

			m_SoundManager.PlaySE_Force("softdrop");
		}
	}

	else if (m_Keyboard.IsKeyJustPressed(KEY_LEFT))
	{
		if (TryMove(-1, 0))
		{
			m_SoundManager.PlaySE_Force("move");
		}
	}

	else if (m_Keyboard.IsKeyJustPressed(KEY_RIGHT))
	{
		if (TryMove(+1, 0))
		{
			m_SoundManager.PlaySE_Force("move");
		}
	}
	else if (m_Keyboard.IsKeyJustPressed(KEY_SPACE))
	{
		HardDrop();
	}
	else if (m_Keyboard.IsKeyJustPressed(KEY_C))
	{
		if (TryHold())
		{
			m_GravityTimer->Restart();
			m_SoundManager.PlaySE_Force("hold");
		}
	}

}

bool SinglePlayState::Exit()
{
	return false;
}

bool SinglePlayState::TrySpawnMino()
{
	// 다음 미노 스폰시 충돌 발생하는지 검사
	Tetromino tempMino(m_Bag->Peek(0));
	tempMino.SetPos(BOARD_WIDTH / 2, 1);

	if (m_Board->IsCollide(tempMino, 0, 0))
	{
		OnGameOver();
		return false;
	}

	m_CurMino = std::make_unique<Tetromino>(m_Bag->Next());

	// 중앙 상단 스폰
	m_CurMino->SetRotation(Tetris::Rotation::R0);
	m_CurMino->SetPos(BOARD_WIDTH / 2, 1);

	m_GravityTimer->Restart();

	return true;
}

bool SinglePlayState::TryMove(int dx, int dy)
{
	if (!m_CurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return false;
	}

	if (m_Board->IsCollide(*m_CurMino, dx, dy))
		return false;

	m_CurMino->SetPos(m_CurMino->GetX() + dx, m_CurMino->GetY() + dy);
	
	return true;
}

bool SinglePlayState::TryRotateCW()
{
	if (!m_CurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return false;
	}

	const Tetris::Rotation prevRot = m_CurMino->GetRotation();
	m_CurMino->RotateCW();

	if (m_Board->IsCollide(*m_CurMino, 0, 0))
	{
		m_CurMino->SetRotation(prevRot);
		return false;
	}

	return true;
}

bool SinglePlayState::TryRotateCCW()
{
	if (!m_CurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return false;
	}

	const Tetris::Rotation prevRot = m_CurMino->GetRotation();
	m_CurMino->RotateCCW();

	if (m_Board->IsCollide(*m_CurMino, 0, 0))
	{
		m_CurMino->SetRotation(prevRot);
		return false;
	}

	return true;
}

bool SinglePlayState::TryHold()
{
	// 한 턴에 한번씩만 홀드 가능
	if (m_bHasHeldThisTurn)
		return false;

	assert(m_CurMino && "m_CurMino is not valid!");
	
	if (m_holdMinoType == Tetris::TetrominoType::None)
	{
		// 현재 미노타입을 홀드 타입으로 대입
		m_holdMinoType = m_CurMino->GetType();

		TrySpawnMino();
	}
	else
	{
		// 홀드 미노와 스왑시 충돌 발생하는지 검사
		Tetromino tempMino(m_holdMinoType);
		tempMino.SetPos(BOARD_WIDTH / 2, 1);

		if (m_Board->IsCollide(tempMino, 0, 0))
		{
			return false;
		}

		auto oldMinoType = m_CurMino->GetType();

		// 현재 미노를 홀드 타입으로 교체 및 초기화
		m_CurMino->SetType(m_holdMinoType);
		m_CurMino->SetRotation(Tetris::Rotation::R0);
		m_CurMino->SetPos(BOARD_WIDTH / 2, 1);

		m_holdMinoType = oldMinoType;
	}

	// 고정되기 전까지 재홀드 불가
	m_bHasHeldThisTurn = true;
	return true;
}

void SinglePlayState::HardDrop()
{
	if (!m_CurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return;
	}

	int dropped = 0;
	while (TryMove(0, +1))
		++dropped;

	if (dropped > 0)
		m_Score->AddHardDrop(dropped);

	LockAndProceed();
	m_SoundManager.PlaySE_Force("harddrop");
}

void SinglePlayState::LockAndProceed()
{
	if (!m_CurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return;
	}

	// 누적 피스갯수 증가
	m_totalPieces++;

	// 보드에 고정
	m_Board->Lock(*m_CurMino);

	// 라인 클리어
	const int cleared = m_Board->ClearFullLines();

	// 점수 기록
	const bool isTSpin = false; // TODO: TSpin 판정 로직 구현필요
	m_Score->OnLinesCleared(cleared, isTSpin);

	// 이번 턴에 콤보 발생하였음
	if (cleared > 0)
	{
		m_bShowCombo = true;
		OnComboAchieved(m_Score->GetCombo());
	}

	// 새 턴 시작되므로 홀드 제한 해제
	m_bHasHeldThisTurn = false;

	// 새 미노 스폰
	TrySpawnMino();

	m_SoundManager.PlaySE_Force("floor");
}


int SinglePlayState::GravityIntervalMS() const
{
	// 레벨에 따라 가속 (ex. 700ms에서 레벨당 50ms 감소, 최소 80ms)
	const int level = std::max(1, m_Score->GetLevel());
	const int base = 700;
	const int step = 50;
	const int ms = base - (level - 1) * step;

	return std::max(80, ms);
}

void SinglePlayState::UpdatePreviewMinos()
{
	for (int i = 0; i < Tetris::MINO_PREVIEW_COUNT; ++i)
		m_PreviewMinos[i] = m_Bag->Peek(i);
}

void SinglePlayState::UpdateGhostMino()
{
	if (!m_CurMino)
		return;

	// 현재 미노를 복사
	m_GhostMino = std::make_unique<Tetromino>(*m_CurMino);

	// 가능한 아래로 이동
	while (!m_Board->IsCollide(*m_GhostMino, 0, +1))
		m_GhostMino->SetPos(m_GhostMino->GetX(), m_GhostMino->GetY() + 1);
}

void SinglePlayState::OnComboAchieved(int comboCount)
{
	std::string combo_string = "combo_" + std::to_string(comboCount);
	m_SoundManager.PlaySE_Force(combo_string);
}

void SinglePlayState::OnGameOver()
{
	m_bGameOver = true;

	m_CurMino = nullptr;
	m_GhostMino = nullptr;

	m_GravityTimer->Stop();
	m_PlayTimer->Pause();
	m_ComboTimer->Stop();
	m_SoftDropTimer->Stop();

	m_SoundManager.StopBGM();
	m_SoundManager.PlaySE_Force("failure");

	// 3초
	m_GameOverTimer->Start();
	m_bWaitingGameOverTransition = true;
}

void SinglePlayState::Shutdown()
{
	m_CurMino.reset();
	m_GhostMino.reset();

	m_Score.reset();
	m_Bag.reset();
	m_Board.reset();

	m_Console.ClearBuffer();
}