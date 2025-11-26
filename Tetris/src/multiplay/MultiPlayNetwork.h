#pragma once

#include "../common/PacketProtocol.h"
#include <memory>

class TetrisClient;
class MultiPlayLogic;

class MultiPlayNetwork
{
public:
    MultiPlayNetwork(std::unique_ptr<TetrisClient> client, MultiPlayLogic& logic);
    ~MultiPlayNetwork();

    // 서버로 송신 (Logic에서 SyncFlag를 기반으로 필요할 때 호출)
    void SyncToServer();

    // 서버에서 수신된 패킷 처리
    void ProcessPackets();

    // Graceful Disconnect
    void SendUnregister();

    bool IsConnected() const;
    int GetPlayerID() const;

private:
    void SendCurMino();
    void SendHold();
    void SendPreview();
    void SendBoard();
    void SendClientGameOver();

private:
    std::unique_ptr<TetrisClient> m_Client;
    MultiPlayLogic& m_Logic;
};