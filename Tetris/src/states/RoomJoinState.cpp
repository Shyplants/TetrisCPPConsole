#include "RoomJoinState.h"
#include "../Console.h"
#include "../inputs/Keyboard.h"
#include "../audio/SoundManager.h"
#include "StateMachine.h"
// #include "MultiPlayState.h"
#include "MultiPlayState.h"
#include "../utils/Logger.h"

// #include "../common/PacketProtocol.hpp"
#include "../network/TetrisClient.h"
#include <nlohmann/json.hpp>

RoomJoinState::RoomJoinState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine)
	: m_Console{ console }
	, m_Keyboard{ keyboard }
	, m_SoundManager{ soundManager }
	, m_StateMachine{ stateMachine }
	, m_Client{ std::make_unique<TetrisClient>() }
{
	// 클라이언트 연결 시도
	if (!m_Client->Connect("127.0.0.1", 60000))
	{
		TETRIS_ERROR("Client Connect Failed!");
	}
}

RoomJoinState::~RoomJoinState()
{
}

void RoomJoinState::OnEnter()
{
	m_Console.ClearBuffer();
	m_sCurrentState = L"Connecting to server...";

	// TryJoinRoom();
}

void RoomJoinState::OnExit()
{
	if (m_Client)
	{
		SendUnregister();

		// 100ms 대기: 패킷 전송 보장
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		m_Client->Disconnect();
	}

	m_Console.ClearBuffer();
}

void RoomJoinState::Update()
{
	HandlePackets();

	if (m_bJoined)
	{
		TETRIS_LOG("Room Join Success!");
	}

	m_Console.ClearBuffer();
}

void RoomJoinState::Draw()
{
	m_Console.Write(20, 10, m_sCurrentState, BRIGHT_WHITE);
}

void RoomJoinState::ProcessInputs()
{
}

bool RoomJoinState::Exit()
{
	return false;
}

void RoomJoinState::TryJoinRoom()
{
	sp::net::message<GameMsg> msgOut;
	msgOut.header.id = GameMsg::Client_RequestRoomJoin;

	m_Client->Send(msgOut);
}

void RoomJoinState::TryTransitionToMultiPlay()
{
	auto nextState = std::make_unique<MultiPlayState>(m_Console, m_Keyboard, m_SoundManager, m_StateMachine, m_bagSeed);
	nextState->SetClient(std::move(m_Client));

	m_StateMachine.PushState(std::move(nextState));
}

void RoomJoinState::HandlePackets()
{
	if (m_Client->IsConnected())
	{
		if (!m_Client->Incoming().empty())
		{
			auto msgIn = m_Client->Incoming().pop_front().msg;

			switch (msgIn.header.id)
			{
				case GameMsg::Server_Ping:
				{
					sp::net::message<GameMsg> pong;
					pong.header.id = GameMsg::Client_Pong;
					m_Client->Send(pong);

					break;
				}

				case GameMsg::Client_Accepted:
				{
					sp::net::message<GameMsg> msgOut;
					msgOut.header.id = GameMsg::Client_RegisterWithServer;

					msgOut << descPlayer;

					m_Client->Send(msgOut);

					m_sCurrentState = L"Connected to server!";
					break;
				}

				case GameMsg::Client_AssignID:
				{
					msgIn >> nPlayerID;

					TETRIS_LOG("Assigned Client ID = " + std::to_string(nPlayerID));
					m_Client->SetPlayerID(nPlayerID);

					TryJoinRoom();
					break;
				}

				case GameMsg::Server_RoomJoinAccepted:
				{
					m_sCurrentState = L"Accepted to RoomJoin!";
					break;
				}

				case GameMsg::Game_SendBagSeed:
				{
					msgIn >> m_bagSeed;

					break;
				}

				case GameMsg::Server_AllPlayersReady:
				{
					TryTransitionToMultiPlay();
					break;
				}

				case GameMsg::Game_AddPlayer:
				{
					sPlayerDescription desc;
					msgIn >> desc;
					m_mapPlayers.insert_or_assign(desc.nUniqueID, desc);

					if (desc.nUniqueID == nPlayerID)
					{
						m_bWatingForConnection = false;
					}

					break;
				}

				case GameMsg::Game_RemovePlayer:
				{
					uint32_t nRemovalID = 0;
					msgIn >> nRemovalID;
					m_mapPlayers.erase(nRemovalID);

					break;
				}

				case GameMsg::Game_UpdatePlayer:
				{
					sPlayerDescription desc;
					msgIn >> desc;
					m_mapPlayers.insert_or_assign(desc.nUniqueID, desc);

					break;
				}
			}
		}
	}
	else
	{
		TETRIS_LOG("Tetris Server Down");
		m_StateMachine.RequestPopDepth(1);
		return;
	}
}

void RoomJoinState::SendUnregister()
{
	if (!m_Client || !m_Client->IsConnected())
	{
		TETRIS_LOG("SendUnregister Failed");
		__debugbreak();
		return;
	}

	sp::net::message<GameMsg> msgOut;
	msgOut.header.id = GameMsg::Client_UnregisterWithServer;
	msgOut << m_Client->GetPlayerID();

	m_Client->Send(msgOut);
}