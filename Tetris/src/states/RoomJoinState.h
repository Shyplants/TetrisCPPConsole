#pragma once

#include "IState.h"
#include <memory>
#include "../common/PacketProtocol.h"

class Console;
class Keyboard;
class SoundManager;
class StateMachine;
class TetrisClient;

class RoomJoinState : public IState
{
public:
    RoomJoinState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine);
    ~RoomJoinState() override;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;
    void Draw() override;
    void ProcessInputs() override;

    bool Exit() override;

private:
    void TryJoinRoom();
    void TryTransitionToMultiPlay();
    void HandlePackets();

private:
    Console& m_Console;
    Keyboard& m_Keyboard;
    SoundManager& m_SoundManager;
    StateMachine& m_StateMachine;

    std::unique_ptr<TetrisClient> m_Client;

    bool m_bJoined{ false };
    bool m_bFailed{ false };

private:
    std::unordered_map<uint32_t, sPlayerDescription> m_mapPlayers = {};
    uint32_t nPlayerID{ 0 };
    sPlayerDescription descPlayer;

    bool m_bWatingForConnection{ true };

    std::wstring m_sCurrentState{};
    uint64_t m_bagSeed{ 0 };
};