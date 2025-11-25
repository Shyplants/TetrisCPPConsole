#include "RoomJoinState.h"
#include "../Console.h"
#include "../inputs/Keyboard.h"
#include "../audio/SoundManager.h"
#include "StateMachine.h"
#include "MultiPlayState.h"
#include "../utils/Logger.h"
#include "../network/TetrisClient.h"

RoomJoinState::RoomJoinState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine)
    : m_Console(console)
    , m_Keyboard(keyboard)
    , m_SoundManager(soundManager)
    , m_StateMachine(stateMachine)
    , m_Client(std::make_unique<TetrisClient>())
{
    // 즉시 Connect 시도 (비동기)
    if (!m_Client->Connect("127.0.0.1", 60000))
    {
        TETRIS_ERROR("Client Connect Failed!");
    }
}

RoomJoinState::~RoomJoinState() {}

void RoomJoinState::OnEnter()
{
    m_Console.ClearBuffer();
    m_sCurrentState = L"Connecting to server...";

    // 타임아웃 측정 시작
    m_ConnectStartTime = std::chrono::steady_clock::now();
}

void RoomJoinState::OnExit()
{
    // 안전한 Unregister 요청
    if (m_Client)
    {
        SendUnregister();
        // TODO: send가 보장되지 않음 -> 개선 필요
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        m_Client->Disconnect();
    }

    m_Console.ClearBuffer();
}

void RoomJoinState::Update()
{
    HandlePackets();

    // 초기 응답을 못 받았을 경우 타임아웃 검사
    if (!m_bServerResponded)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_ConnectStartTime).count();

        if (elapsed > SERVER_RESPONSE_TIMEOUT_MS)
        {
            m_sCurrentState = L"Server did not respond...";
            m_StateMachine.RequestPopDepth(1);
            return;
        }
    }

    // 상태 전이는 Update 마지막에서만 안전하게 처리
    if (m_bTransitionQueued)
    {
        PerformTransitionToMultiPlay();
        return;
    }
}

void RoomJoinState::Draw()
{
    m_Console.Write(20, 10, m_sCurrentState, BRIGHT_WHITE);
}

void RoomJoinState::ProcessInputs() {}

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

void RoomJoinState::ScheduleTransitionToMultiPlay()
{
    m_bTransitionQueued = true;     // 즉시 PushState 하지 않고 예약만 한다.
}

void RoomJoinState::PerformTransitionToMultiPlay()
{
    m_bTransitionQueued = false;

    auto nextState = std::make_unique<MultiPlayState>(
        m_Console, m_Keyboard, m_SoundManager, m_StateMachine, m_BagSeed);

    // Client 소유권 넘기기
    nextState->SetClient(std::move(m_Client));

    m_StateMachine.PushState(std::move(nextState));
}

void RoomJoinState::HandlePackets()
{
    if (!m_Client->IsConnected())
        return;

    while (!m_Client->Incoming().empty())
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
                m_bServerResponded = true;

                // Register 요청
                sp::net::message<GameMsg> msgOut;
                msgOut.header.id = GameMsg::Client_RegisterWithServer;
                m_Client->Send(msgOut);

                m_sCurrentState = L"Connected to server!";
                break;
            }

            case GameMsg::Client_AssignID:
            {
                msgIn >> m_PlayerID;
                m_Client->SetPlayerID(m_PlayerID);
                TryJoinRoom();
                break;
            }

            case GameMsg::Server_RoomJoinAccepted:
            {
                m_sCurrentState = L"Joined room!";
                break;
            }

            case GameMsg::Game_SendBagSeed:
            {
                msgIn >> m_BagSeed;
                break;
            }

            case GameMsg::Server_AllPlayersReady:
            {
                // 즉시 전이 금지 → 예약
                ScheduleTransitionToMultiPlay();
                break;
            }

            default:
            {
                TETRIS_ERROR("GameMsg Not Valid!");
                __debugbreak();
                break;
            }
        }
    }
}

void RoomJoinState::SendUnregister()
{
    if (!m_Client || !m_Client->IsConnected())
        return;

    sp::net::message<GameMsg> msgOut;
    msgOut.header.id = GameMsg::Client_UnregisterWithServer;
    msgOut << m_Client->GetPlayerID();
    m_Client->Send(msgOut);
}
