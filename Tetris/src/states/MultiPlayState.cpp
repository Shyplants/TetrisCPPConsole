#include "MultiPlayState.h"
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
#include "../network/TetrisClient.h"
#include "GameOverState.h"


MultiPlayState::MultiPlayState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine, uint64_t bagSeed)
	: m_Console{ console }
	, m_Keyboard{ keyboard }
	, m_SoundManager{ soundManager }
	, m_StateMachine{ stateMachine }
	, m_MyBoard{ std::make_unique<Board>() }
	, m_EnemyBoard{ std::make_unique<Board>() }
	, m_MyBag{ std::make_unique<BagRandom>() }
	, m_EnemyBag{ std::make_unique<BagRandom>() }
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
	, m_MyRenderer{ std::make_unique<ConsoleRenderer>(console, m_BoardLeft - 30, m_BoardTop) }
	, m_EnemyRenderer{ std::make_unique<ConsoleRenderer>(console, m_BoardLeft + 30, m_BoardTop) }
{
	m_MyBag->Seed(bagSeed);
	m_EnemyBag->Seed(bagSeed);
}

MultiPlayState::~MultiPlayState()
{
}

void MultiPlayState::OnEnter()
{
	m_Console.ClearBuffer();

	m_SoundManager.PlayBGM("play_bgm");

	m_GravityTimer->Start();
	m_PlayTimer->Start();
	m_ComboTimer->Start();
	m_SoftDropTimer->Start();

	// 미노 스폰
	TrySpawnMino();
	m_EnemyCurMino = std::make_unique<Tetromino>();
}

void MultiPlayState::OnExit()
{
	if (m_Client)
	{
		m_Client->Disconnect();
	}
}

void MultiPlayState::Update()
{
	if (m_bWaitingGameOverTransition)
	{
		if (m_GameOverTimer->ElapsedMS() >= 3000) // 3초
		{
			m_bWaitingGameOverTransition = false;

			// GameOverState로 전이
			m_StateMachine.PushState(std::make_unique<GameOverState>(m_Console, m_Keyboard, m_SoundManager, m_StateMachine, m_bIsVictory));
		}
	}

	if (m_bServerGameOver)
	{
		return;
	}

	HandlePackets();


	if (m_bClientGameOver)
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

	// 고스트 미노 업데이트
	UpdateMyGhostMino();

	// 미리보기 미노 업데이트
	UpdateMyPreviewMinos();

	if (!m_bClientReady)
		m_bClientReady = true;

	
}

void MultiPlayState::Draw()
{
	if (!m_bClientReady || !m_bRemoteReady[0] || !m_bRemoteReady[1])
		return;

	m_Console.ClearBuffer();

	m_MyRenderer->DrawBoard(*m_MyBoard, m_MyCurMino.get(), m_MyGhostMino.get());
	m_MyRenderer->DrawPreviewPanel(m_MyPreviewMinos);
	m_MyRenderer->DrawHoldPanel(m_MyHoldMinoType);
	m_MyRenderer->DrawInfoPanel(*m_Score, *m_PlayTimer, m_totalPieces, m_LastCombo, m_bShowCombo, *m_ComboTimer);


	m_EnemyRenderer->DrawBoard(*m_EnemyBoard, m_EnemyCurMino.get(), m_EnemyGhostMino.get());
	m_EnemyRenderer->DrawPreviewPanel(m_EnemyPreviewMinos);
	m_EnemyRenderer->DrawHoldPanel(m_EnemyHoldMinoType);
}

void MultiPlayState::ProcessInputs()
{
	if (m_bClientGameOver || m_bServerGameOver)
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

			SendHoldMinoState();
		}
	}
}

bool MultiPlayState::Exit()
{
	return false;
}

void MultiPlayState::SetClient(std::unique_ptr<TetrisClient> client)
{
	m_Client = std::move(client);
}

void MultiPlayState::DrawBoards()
{
}

void MultiPlayState::DrawInfoPanels()
{
}

bool MultiPlayState::TrySpawnMino()
{
	// 다음 미노 스폰시 충돌 발생하는지 검사
	Tetromino tempMino(m_MyBag->Peek(0));
	tempMino.SetPos(BOARD_WIDTH / 2, 1);

	if (m_MyBoard->IsCollide(tempMino, 0, 0))
	{
		OnClientGameOver();
		return false;
	}

	m_MyCurMino = std::make_unique<Tetromino>(m_MyBag->Next());

	// 중앙 상단 스폰
	m_MyCurMino->SetRotation(Tetris::Rotation::R0);
	m_MyCurMino->SetPos(BOARD_WIDTH / 2, 1);

	m_GravityTimer->Restart();

	SendPreviewMinoState();
}

bool MultiPlayState::TryMove(int dx, int dy)
{
	if (!m_MyCurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return false;
	}

	if (m_MyBoard->IsCollide(*m_MyCurMino, dx, dy))
		return false;

	m_MyCurMino->SetPos(m_MyCurMino->GetX() + dx, m_MyCurMino->GetY() + dy);

	SendCurMinoState();
	return true;
}

bool MultiPlayState::TryRotateCW()
{
	if (!m_MyCurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return false;
	}

	const Tetris::Rotation prevRot = m_MyCurMino->GetRotation();
	m_MyCurMino->RotateCW();

	if (m_MyBoard->IsCollide(*m_MyCurMino, 0, 0))
	{
		m_MyCurMino->SetRotation(prevRot);
		return false;
	}

	SendCurMinoState();
	return true;
}

bool MultiPlayState::TryRotateCCW()
{
	if (!m_MyCurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return false;
	}

	const Tetris::Rotation prevRot = m_MyCurMino->GetRotation();
	m_MyCurMino->RotateCCW();

	if (m_MyBoard->IsCollide(*m_MyCurMino, 0, 0))
	{
		m_MyCurMino->SetRotation(prevRot);
		return false;
	}

	SendCurMinoState();
	return true;
}

bool MultiPlayState::TryHold()
{
	// 한 턴에 한번씩만 홀드 가능
	if (m_bHasHeldThisTurn)
		return false;

	assert(m_MyCurMino && "m_CurMino is not valid!");

	if (m_MyHoldMinoType == Tetris::TetrominoType::None)
	{
		// 현재 미노타입을 홀드 타입으로 대입
		m_MyHoldMinoType = m_MyCurMino->GetType();

		TrySpawnMino();
	}
	else
	{
		// 홀드 미노와 스왑시 충돌 발생하는지 검사
		Tetromino tempMino(m_MyHoldMinoType);
		tempMino.SetPos(BOARD_WIDTH / 2, 1);

		if (m_MyBoard->IsCollide(tempMino, 0, 0))
		{
			return false;
		}

		auto oldMinoType = m_MyCurMino->GetType();

		// 현재 미노를 홀드 타입으로 교체 및 초기화
		m_MyCurMino->SetType(m_MyHoldMinoType);
		m_MyCurMino->SetRotation(Tetris::Rotation::R0);
		m_MyCurMino->SetPos(BOARD_WIDTH / 2, 1);

		m_MyHoldMinoType = oldMinoType;
	}

	// 고정되기 전까지 재홀드 불가
	m_bHasHeldThisTurn = true;
	return true;
}

void MultiPlayState::HardDrop()
{
	if (!m_MyCurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return;
	}

	int dropped = 0;
	while (TryMove(0, 1))
		++dropped;

	if (dropped > 0)
		m_Score->AddHardDrop(dropped);

	LockAndProceed();
	m_SoundManager.PlaySE_Force("harddrop");
}

void MultiPlayState::LockAndProceed()
{
	if (!m_MyCurMino)
	{
		TETRIS_LOG("m_CurMino is not valid!");
		return;
	}

	// 누적 피스갯수 증가
	m_totalPieces++;

	// 보드에 고정
	m_MyBoard->Lock(*m_MyCurMino);

	// 라인 클리어
	const int cleared = m_MyBoard->ClearFullLines();

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

	SendBoardState();
}


int MultiPlayState::GravityIntervalMS() const
{
	// 레벨에 따라 가속 (ex. 700ms에서 레벨당 50ms 감소, 최소 80ms)
	const int level = std::max(1, m_Score->GetLevel());
	const int base = 700;
	const int step = 50;
	const int ms = base - (level - 1) * step;

	return std::max(80, ms);
}

void MultiPlayState::UpdateMyGhostMino()
{
	if (!m_MyCurMino)
		return;

	// 현재 미노를 복사
	m_MyGhostMino = std::make_unique<Tetromino>(*m_MyCurMino);

	// 가능한 아래로 이동
	while (!m_MyBoard->IsCollide(*m_MyGhostMino, 0, +1))
		m_MyGhostMino->SetPos(m_MyGhostMino->GetX(), m_MyGhostMino->GetY() + 1);
}

void MultiPlayState::UpdateEnemyGhostMino()
{
	if (!m_EnemyCurMino)
		return;

	// 현재 미노를 복사
	m_EnemyGhostMino = std::make_unique<Tetromino>(*m_EnemyCurMino);

	// 가능한 아래로 이동
	while (!m_EnemyBoard->IsCollide(*m_EnemyGhostMino, 0, +1))
		m_EnemyGhostMino->SetPos(m_EnemyGhostMino->GetX(), m_EnemyGhostMino->GetY() + 1);
}

void MultiPlayState::UpdateMyPreviewMinos()
{
	for (int i = 0; i < Tetris::MINO_PREVIEW_COUNT; ++i)
		m_MyPreviewMinos[i] = m_MyBag->Peek(i);
	
}

void MultiPlayState::OnComboAchieved(int comboCount)
{
	std::string combo_string = "combo_" + std::to_string(comboCount);
	m_SoundManager.PlaySE_Force(combo_string);
}

void MultiPlayState::OnClientGameOver()
{
	m_bClientGameOver = true;

	m_MyCurMino = nullptr;
	m_MyGhostMino = nullptr;

	// 서버에 Game_PlayerDead 패킷 전송
	SendClientGameOver();
}

void MultiPlayState::SendCurMinoState()
{
	if (!m_Client || !m_MyCurMino)
	{
		TETRIS_LOG("SendCurMinoState Failed");
		__debugbreak();
	}

	sMinoState state;
	state.type = static_cast<int32_t>(m_MyCurMino->GetType());
	state.x = m_MyCurMino->GetX();
	state.y = m_MyCurMino->GetY();
	state.rot = static_cast<int32_t>(m_MyCurMino->GetRotation());

	sp::net::message<GameMsg> msgOut;
	msgOut.header.id = GameMsg::Game_CurMinoState;
	msgOut << state;

	m_Client->Send(msgOut);
}

void MultiPlayState::SendHoldMinoState()
{
	if (!m_Client || (m_MyHoldMinoType == Tetris::TetrominoType::None))
	{
		TETRIS_LOG("SendHoldMinoState Failed");
		__debugbreak();
	}

	sMinoState state;
	state.type = static_cast<int32_t>(m_MyHoldMinoType);

	sp::net::message<GameMsg> msgOut;
	msgOut.header.id = GameMsg::Game_HoldMinoState;
	msgOut << state;

	m_Client->Send(msgOut);
}

void MultiPlayState::SendPreviewMinoState()
{
	if (!m_Client || !m_MyBag)
	{
		TETRIS_LOG("SendPreviewMinoState Failed");
		__debugbreak();
	}

	sPreviewMinoState state;
	for (int i = 0; i < Tetris::MINO_PREVIEW_COUNT; ++i)
		state.previewTypes[i] = static_cast<int32_t>(m_MyBag->Peek(i));
	
	sp::net::message<GameMsg> msgOut;
	msgOut.header.id = GameMsg::Game_PreviewMinoState;
	msgOut << state;

	m_Client->Send(msgOut);
}

void MultiPlayState::SendBoardState()
{
	if (!m_Client || !m_MyBoard)
	{
		TETRIS_LOG("SendBoardState Failed");
		__debugbreak();
	}

	sBoardState state = m_MyBoard->ToPacket();

	sp::net::message<GameMsg> msgOut;
	msgOut.header.id = GameMsg::Game_BoardState;

	msgOut << state;

	m_Client->Send(msgOut);
}

void MultiPlayState::SendClientGameOver()
{
	sp::net::message<GameMsg> msgOut;
	msgOut.header.id = GameMsg::Game_PlayerDead;
	msgOut << m_Client->GetPlayerID();

	m_Client->Send(msgOut);
}

void MultiPlayState::OnServerGameOver()
{
	m_bServerGameOver = true;

	m_GravityTimer->Stop();
	m_PlayTimer->Pause();
	m_ComboTimer->Stop();
	m_SoftDropTimer->Stop();

	m_SoundManager.StopBGM();

	if (m_bIsVictory)
	{
		m_SoundManager.PlaySE_Force("failure"); // TODO: 이겼을 땐 승리 효과음
	}
	else
	{
		m_SoundManager.PlaySE_Force("failure");
	}

	m_GameOverTimer->Start();
	m_bWaitingGameOverTransition = true;
}


void MultiPlayState::HandlePackets()
{
	if (m_Client->IsConnected())
	{
		if (!m_Client->Incoming().empty())
		{
			auto msgIn = m_Client->Incoming().pop_front().msg;

			switch (msgIn.header.id)
			{
				case GameMsg::Game_CurMinoState:
				{
					sMinoState state;
					msgIn >> state;

					// m_EnemyCurMino = std::make_unique<Tetromino>(static_cast<Tetris::TetrominoType>(state.type));
					m_EnemyCurMino->SetType(static_cast<Tetris::TetrominoType>(state.type));
					m_EnemyCurMino->SetPos(state.x, state.y);
					m_EnemyCurMino->SetRotation(static_cast<Tetris::Rotation>(state.rot));

					UpdateEnemyGhostMino();

					if (!m_bRemoteReady[0])
						m_bRemoteReady[0] = true;

					break;
				}

				case GameMsg::Game_HoldMinoState:
				{
					sMinoState state;
					msgIn >> state;

					m_EnemyHoldMinoType = static_cast<Tetris::TetrominoType>(state.type);
					break;
				}

				case GameMsg::Game_PreviewMinoState:
				{
					sPreviewMinoState state;
					msgIn >> state;

					for (int i = 0; i < Tetris::MINO_PREVIEW_COUNT; ++i)
						m_EnemyPreviewMinos[i] = static_cast<Tetris::TetrominoType>(state.previewTypes[i]);

					if (!m_bRemoteReady[1])
						m_bRemoteReady[1] = true;

					break;
				}

				case GameMsg::Game_BoardState:
				{
					sBoardState state;
					msgIn >> state;

					for (int i = 0; i < state.cells.size(); ++i)
					{
						int x = i % BOARD_WIDTH;
						int y = i / BOARD_WIDTH;
						m_EnemyBoard->Set(x, y, state.cells[i]);
					}
					break;
				}

				case GameMsg::Server_GameOver:
				{
					sGameOverInfo info;
					msgIn >> info;

					m_bIsVictory = info.nWinnerID == m_Client->GetPlayerID();
					OnServerGameOver();
					break;
				}
			}
		}
	}
	else
	{
		TETRIS_LOG("Tetris Server Down");
	}
}