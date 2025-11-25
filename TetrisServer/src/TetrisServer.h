#pragma once

#include "common/PacketProtocol.h"

// -----------------------------
// 이벤트 타입
// -----------------------------
enum class ServerEventType
{
    ClientValidated,
    ClientDisconnected,
    MessageReceived
};

// -----------------------------
// 메인스레드에서 처리할 이벤트 데이터
// -----------------------------
struct ServerEvent
{
    ServerEventType type{};
    uint32_t clientID{};                       // 클라이언트 식별자
    sp::net::message<GameMsg> msg{};           // 메시지 (MessageReceived일 때 사용)
};

// -----------------------------
// 서버 본체
// -----------------------------
class TetrisServer : public sp::net::server_interface<GameMsg>
{
public:
    TetrisServer(uint16_t nPort);

protected:
    // ASIO 스레드 콜백
    bool OnClientConnect(std::shared_ptr<sp::net::connection<GameMsg>> client) override;
    void OnClientDisconnect(std::shared_ptr<sp::net::connection<GameMsg>> client) override;
    void OnMessage(std::shared_ptr<sp::net::connection<GameMsg>> client,
        sp::net::message<GameMsg>& msg) override;
    void OnClientValidated(std::shared_ptr<sp::net::connection<GameMsg>> client) override;

    // 메인스레드 주기적 실행
    void OnUpdate() override;

private:
    // 메인 스레드 처리 함수
    void ProcessEvents();
    void ProcessPingPong();

    void HandleClientValidated(uint32_t id);
    void HandleClientDisconnected(uint32_t id);
    void HandleMessage(uint32_t clientID, sp::net::message<GameMsg>& msg);

    bool IsClientValidated(uint32_t id) const;

    std::shared_ptr<sp::net::connection<GameMsg>>
        GetConnectionByID(uint32_t id)
    {
        for (auto& c : m_deqConnections)
        {
            if (c && c->GetID() == id)
                return c;
        }
        return nullptr;
    }

private:
    // 게임 로직 관련
    uint64_t GenerateSeed();
    void PingAllClients();
    void CheckTimeouts();

private:
    // 인증된 클라이언트 목록
    std::unordered_set<uint32_t> m_ValidatedClients;

    // 게임중인 플레이어
    std::unordered_map<uint32_t, sPlayerDescription> m_mapConnectedPlayers;
    std::vector<uint32_t> m_vGarbageIDs;

    // ASIO → Main Thread 이벤트 큐
    std::mutex m_EventMutex;
    std::deque<ServerEvent> m_EventQueue;

    // Ping/Pong
    std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> m_LastPongTime;

    static constexpr int PING_INTERVAL_MS = 2000;
    static constexpr int PONG_TIMEOUT_MS = 5000;

    std::chrono::steady_clock::time_point m_LastPingTime{};
};
