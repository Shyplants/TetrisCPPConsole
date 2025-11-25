#include "TetrisServer.h"
#include <iostream>
#include <random>

TetrisServer::TetrisServer(uint16_t nPort)
    : sp::net::server_interface<GameMsg>(nPort)
{
    m_LastPingTime = std::chrono::steady_clock::now();
}

bool TetrisServer::OnClientConnect(std::shared_ptr<sp::net::connection<GameMsg>>)
{
    return true;    // 접속 허용
}

void TetrisServer::OnClientDisconnect(std::shared_ptr<sp::net::connection<GameMsg>> client)
{
    if (!client)
        return;

    uint32_t id = client->GetID();

    // 연결 해제 이벤트 → 메인 스레드로 전달
    {
        std::lock_guard<std::mutex> lock(m_EventMutex);
        m_EventQueue.push_back({ ServerEventType::ClientDisconnected, id });
    }
}

void TetrisServer::OnMessage(std::shared_ptr<sp::net::connection<GameMsg>> client,
    sp::net::message<GameMsg>& msg)
{
    uint32_t id = client->GetID();

    // 메시지 수신 이벤트 → 메인 스레드로 전달
    {
        std::lock_guard<std::mutex> lock(m_EventMutex);
        m_EventQueue.push_back({ ServerEventType::MessageReceived, id, msg });
    }
}

void TetrisServer::OnClientValidated(std::shared_ptr<sp::net::connection<GameMsg>> client)
{
    uint32_t id = client->GetID();

    // 이벤트 push
    {
        std::lock_guard<std::mutex> lock(m_EventMutex);
        m_EventQueue.push_back({ ServerEventType::ClientValidated, id });
    }

    // 클라이언트에게 Accepted 패킷 송신 (ASIO 스레드)
    sp::net::message<GameMsg> msg;
    msg.header.id = GameMsg::Client_Accepted;
    client->Send(msg);
}

void TetrisServer::OnUpdate()
{
    ProcessEvents();    // 메인 스레드에서 이벤트 처리
    ProcessPingPong();  // Ping/Pong 처리
}

// =====================================================
// 이벤트 큐 처리 (메인 스레드)
// =====================================================
void TetrisServer::ProcessEvents()
{
    std::deque<ServerEvent> events;

    // ASIO 큐 → 임시 버퍼로 swap
    {
        std::lock_guard<std::mutex> lock(m_EventMutex);
        events.swap(m_EventQueue);
    }

    for (auto& ev : events)
    {
        switch (ev.type)
        {
        case ServerEventType::ClientValidated:
            HandleClientValidated(ev.clientID);
            break;

        case ServerEventType::ClientDisconnected:
            HandleClientDisconnected(ev.clientID);
            break;

        case ServerEventType::MessageReceived:
            HandleMessage(ev.clientID, ev.msg);
            break;
        }
    }
}

// =====================================================
// 클라이언트 인증 완료
// =====================================================
void TetrisServer::HandleClientValidated(uint32_t id)
{
    m_ValidatedClients.insert(id);
    m_LastPongTime[id] = std::chrono::steady_clock::now();

    std::cout << "[Validated] ID = " << id << "\n";
}

// =====================================================
// 연결 해제 처리 (메인 스레드)
// =====================================================
void TetrisServer::HandleClientDisconnected(uint32_t id)
{
    m_LastPongTime.erase(id);

    bool inGame = (m_mapConnectedPlayers.size() == 2);

    if (inGame)
    {
        uint32_t loser = id, winner = 0;

        for (auto& kv : m_mapConnectedPlayers)
        {
            if (kv.first != loser)
            {
                winner = kv.first;
                break;
            }
        }

        sp::net::message<GameMsg> msgOut;
        msgOut.header.id = GameMsg::Server_GameOver;

        sGameOverInfo info{ winner, loser };
        msgOut << info;

        MessageAllClients(msgOut);
    }

    m_ValidatedClients.erase(id);
    m_mapConnectedPlayers.erase(id);
    m_vGarbageIDs.push_back(id);

    std::cout << "[Disconnected] ID = " << id << "\n";
}

// =====================================================
// 수신 메시지 처리 (메인 스레드)
// =====================================================
void TetrisServer::HandleMessage(uint32_t clientID, sp::net::message<GameMsg>& msg)
{
    auto client = GetConnectionByID(clientID);
    if (!client)
    {
        std::cout << "[ERROR] Message from invalid clientID=" << clientID << "\n";
        return;
    }

    switch (msg.header.id)
    {
    case GameMsg::Client_RegisterWithServer:
    {
        sPlayerDescription desc;
        msg >> desc;
        desc.nUniqueID = clientID;

        m_mapConnectedPlayers[clientID] = desc;

        sp::net::message<GameMsg> out;
        out.header.id = GameMsg::Client_AssignID;
        out << clientID;

        MessageClient(client, out);
        break;
    }

    case GameMsg::Client_Pong:
    {
        m_LastPongTime[clientID] = std::chrono::steady_clock::now();
        std::cout << "[Pong] " << clientID << "\n";
        break;
    }

    case GameMsg::Client_RequestRoomJoin:
    {
        {
            sp::net::message<GameMsg> out;
            out.header.id = GameMsg::Server_RoomJoinAccepted;

            MessageClient(client, out);
        }

        if (m_mapConnectedPlayers.size() == 2)
        {
            {
                sp::net::message<GameMsg> out;
                out.header.id = GameMsg::Game_SendBagSeed;
                out << GenerateSeed();

                MessageAllClients(out);
            }

            {
                sp::net::message<GameMsg> out;
                out.header.id = GameMsg::Server_AllPlayersReady;

                MessageAllClients(out);
            }
        }

        break;
    }

    case GameMsg::Game_PlayerDead:
    {
        uint32_t loserID;
        msg >> loserID;

        if (m_mapConnectedPlayers.size() == 2)
        {
            uint32_t winnerID = 0;
            for (auto& kv : m_mapConnectedPlayers)
            {
                if (kv.first != loserID)
                {
                    winnerID = kv.first;
                    break;
                }
            }

            sp::net::message<GameMsg> out;
            out.header.id = GameMsg::Server_GameOver;

            sGameOverInfo info{ winnerID, loserID };
            out << info;

            MessageAllClients(out);
        }
        break;
    }

    case GameMsg::Game_CurMinoState:
    case GameMsg::Game_PreviewMinoState:
    case GameMsg::Game_HoldMinoState:
    case GameMsg::Game_BoardState:
    case GameMsg::Game_UpdatePlayer:
    {
        MessageAllClients(msg, client);

        break;
    }


    default:
        std::cout << "[Unvalid GameMsg] " << (uint32_t)msg.header.id << "\n";
        __debugbreak();
        break;
    }
}

// =====================================================
// Ping / Pong
// =====================================================
void TetrisServer::ProcessPingPong()
{
    auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastPingTime).count()
        > PING_INTERVAL_MS)
    {
        PingAllClients();
    }

    CheckTimeouts();
}

uint64_t TetrisServer::GenerateSeed()
{
    uint64_t rd = ((uint64_t)std::random_device{}() << 32)
        ^ (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count()
        ^ (uint64_t)std::hash<std::thread::id>()(std::this_thread::get_id());

    return rd;
}

void TetrisServer::PingAllClients()
{
    sp::net::message<GameMsg> msgOut;
    msgOut.header.id = GameMsg::Server_Ping;

    for (auto& client : m_deqConnections)
    {
        if (!client || !client->IsConnected())
            continue;

        uint32_t id = client->GetID();

        if (IsClientValidated(id))
            client->Send(msgOut);
    }

    m_LastPingTime = std::chrono::steady_clock::now();
}

void TetrisServer::CheckTimeouts()
{
    auto now = std::chrono::steady_clock::now();

    for (auto it = m_LastPongTime.begin(); it != m_LastPongTime.end();)
    {
        uint32_t id = it->first;

        if (!IsClientValidated(id))
        {
            ++it;
            continue;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();

        if (elapsed > PONG_TIMEOUT_MS)
        {
            std::cout << "[Timeout] " << id << "\n";

            auto conn = GetConnectionByID(id);
            if (conn)
                conn->Disconnect();

            it = m_LastPongTime.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool TetrisServer::IsClientValidated(uint32_t id) const
{
    return m_ValidatedClients.count(id) > 0;
}
