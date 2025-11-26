#pragma once

#include "IState.h"
#include <memory>

// 전방 선언
class Console;
class Keyboard;
class SoundManager;
class StateMachine;

class MultiPlayLogic;
class MultiPlayNetwork;
class MultiPlayRenderer;
class TetrisClient;
class Timer;

class MultiPlayState final : public IState
{
public:
    MultiPlayState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine, std::unique_ptr<TetrisClient> client, uint64_t bagSeed);
    ~MultiPlayState() override;

    void OnEnter() override;
    void OnExit() override;

    void Update() override;
    void Draw() override;
    void ProcessInputs() override;

    bool Exit() override;

private:
    void CheckGameOverTransition();
    void PlaySoundEffects();

private:
    Console& m_Console;
    Keyboard& m_Keyboard;
    SoundManager& m_SoundManager;
    StateMachine& m_StateMachine;

    std::unique_ptr<MultiPlayLogic>    m_Logic;
    std::unique_ptr<MultiPlayNetwork>  m_Network;
    std::unique_ptr<MultiPlayRenderer> m_Renderer;

    bool m_bWaitingGameOverTransition{ false };
    bool m_bIsVictory{ false };
    uint64_t m_bagSeed = 0;

    std::unique_ptr<TetrisClient> m_Client;

    std::unique_ptr<Timer> m_GameOverTimer;
    std::unique_ptr<Timer> m_SoftDropTimer;
};
