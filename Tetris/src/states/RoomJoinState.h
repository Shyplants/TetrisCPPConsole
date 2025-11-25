#pragma once

#include "IState.h"
#include <memory>
#include <chrono>
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
    void TryJoinRoom();                 // 서버에 RoomJoin 요청
    void HandlePackets();               // 서버로부터 온 메시지 처리
    void SendUnregister();              // 상태 종료 시 서버에 Unregister

    void ScheduleTransitionToMultiPlay();   // transition 지연 예약
    void PerformTransitionToMultiPlay();    // 실제 상태 전이

private:
    Console& m_Console;
    Keyboard& m_Keyboard;
    SoundManager& m_SoundManager;
    StateMachine& m_StateMachine;

    std::unique_ptr<TetrisClient> m_Client;

    // 연결 과정 상태 표시
    std::wstring m_sCurrentState{};

    // 서버로부터 받은 플레이어 ID
    uint32_t m_PlayerID{ 0 };

    // 시드 (멀티플레이 시작 시 필요)
    uint64_t m_BagSeed{ 0 };

    // 상태 전이 예약 플래그
    bool m_bTransitionQueued{ false };

    // 서버 응답 타임아웃용
    bool m_bServerResponded{ false };
    std::chrono::steady_clock::time_point m_ConnectStartTime{};
    static constexpr int SERVER_RESPONSE_TIMEOUT_MS = 5000; // 5초
};

