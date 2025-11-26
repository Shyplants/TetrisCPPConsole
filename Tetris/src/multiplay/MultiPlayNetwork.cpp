#include "MultiPlayNetwork.h"
#include "../network/TetrisClient.h"
#include "../utils/Logger.h"
#include "../common/PacketProtocol.h"
#include "../common/TetrisTypes.h"
#include "MultiPlayLogic.h"
#include "../Tetromino.h"
#include "../Board.h"

using namespace Tetris;

MultiPlayNetwork::MultiPlayNetwork(std::unique_ptr<TetrisClient> client, MultiPlayLogic& logic)
    : m_Client(std::move(client))
    , m_Logic(logic)
{
}

MultiPlayNetwork::~MultiPlayNetwork() = default;

bool MultiPlayNetwork::IsConnected() const
{
    return m_Client && m_Client->IsConnected();
}

int MultiPlayNetwork::GetPlayerID() const
{
    return m_Client ? m_Client->GetPlayerID() : -1;
}

void MultiPlayNetwork::SyncToServer()
{
    if (!IsConnected())
        return;

    // Local CurMino 변경됨
    if (m_Logic.ShouldSyncCurMino())
        SendCurMino();

    // Local Hold 변경됨
    if (m_Logic.ShouldSyncHold())
        SendHold();

    // Local Preview 변경됨
    if (m_Logic.ShouldSyncPreview())
        SendPreview();

    // Local Board 변경됨 (Lock & Proceed)
    if (m_Logic.ShouldSyncBoard())
        SendBoard();

    m_Logic.ClearSyncFlags();
}

void MultiPlayNetwork::ProcessPackets()
{
    if (!IsConnected())
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

        case GameMsg::Game_CurMinoState:
        {
            sMinoState state;
            msgIn >> state;
            m_Logic.ApplyEnemyMinoState(state);
            break;
        }

        case GameMsg::Game_HoldMinoState:
        {
            sMinoState state;
            msgIn >> state;
            m_Logic.ApplyEnemyHoldState(
                static_cast<Tetris::TetrominoType>(state.type));
            break;
        }

        case GameMsg::Game_PreviewMinoState:
        {
            sPreviewMinoState state;
            msgIn >> state;
            m_Logic.ApplyEnemyPreviewState(state);
            break;
        }

        case GameMsg::Game_BoardState:
        {
            sBoardState state;
            msgIn >> state;
            m_Logic.ApplyEnemyBoardState(state);
            break;
        }

        case GameMsg::Game_PlayerDead:
        {
            sGameOverInfo info;
            msgIn >> info;

            // 상대가 죽었는지 Local이 죽었는지 판별 필요 없음:
            // 서버가 Winner/Loser 알려줌
            // (이 로직은 MultiPlayState에서 처리)
            break;
        }

        case GameMsg::Server_GameOver:
        {
            sGameOverInfo info;
            msgIn >> info;

            bool isLocalWinner =
                info.nWinnerID == m_Client->GetPlayerID();

            // Logic에는 GameOver 여부만 반영
            if (isLocalWinner)
                m_Logic.SetGameOver(PlayerSide::Remote);
            else
                m_Logic.SetGameOver(PlayerSide::Local);

            break;
        }
        }
    }
}

void MultiPlayNetwork::SendCurMino()
{
    Tetromino* cur = m_Logic.GetCurMino(PlayerSide::Local);
    if (!cur)
        return;

    sMinoState state;
    state.type = static_cast<int32_t>(cur->GetType());
    state.x = cur->GetX();
    state.y = cur->GetY();
    state.rot = static_cast<int32_t>(cur->GetRotation());

    sp::net::message<GameMsg> msgOut;
    msgOut.header.id = GameMsg::Game_CurMinoState;
    msgOut << state;

    m_Client->Send(msgOut);
}

void MultiPlayNetwork::SendHold()
{
    sMinoState state;
    state.type = static_cast<int32_t>(m_Logic.GetHoldType(PlayerSide::Local));
        
    sp::net::message<GameMsg> msgOut;
    msgOut.header.id = GameMsg::Game_HoldMinoState;
    msgOut << state;

    m_Client->Send(msgOut);
}

void MultiPlayNetwork::SendPreview()
{
    sPreviewMinoState state;
    const auto& preview = m_Logic.GetPreview(PlayerSide::Local);

    for (int i = 0; i < Tetris::MINO_PREVIEW_COUNT; ++i)
        state.previewTypes[i] = static_cast<int32_t>(preview[i]);

    sp::net::message<GameMsg> msgOut;
    msgOut.header.id = GameMsg::Game_PreviewMinoState;
    msgOut << state;

    m_Client->Send(msgOut);
}

void MultiPlayNetwork::SendBoard()
{
    sBoardState state = m_Logic.GetBoard(PlayerSide::Local)->ToPacket();
        
    sp::net::message<GameMsg> msgOut;
    msgOut.header.id = GameMsg::Game_BoardState;
    msgOut << state;

    m_Client->Send(msgOut);
}

void MultiPlayNetwork::SendClientGameOver()
{
    sp::net::message<GameMsg> msgOut;
    msgOut.header.id = GameMsg::Game_PlayerDead;
    msgOut << GetPlayerID();

    m_Client->Send(msgOut);
}

void MultiPlayNetwork::SendUnregister()
{
    if (!IsConnected())
        return;

    sp::net::message<GameMsg> msgOut;
    msgOut.header.id = GameMsg::Client_UnregisterWithServer;
    msgOut << GetPlayerID();

    m_Client->Send(msgOut);
}
