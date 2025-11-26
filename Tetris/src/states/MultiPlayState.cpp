#include "MultiPlayState.h"
#include "../Console.h"
#include "../inputs/Keyboard.h"
#include "../audio/SoundManager.h"
#include "../utils/Timer.h"
#include "../utils/Logger.h"
#include "StateMachine.h"

#include "../network/TetrisClient.h"
#include "GameOverState.h"
#include "../GameConfig.h"

#include "../multiplay/MultiPlayLogic.h"
#include "../multiplay/MultiPlayNetwork.h"
#include "../multiplay/MultiPlayRenderer.h"

using namespace Tetris;

MultiPlayState::MultiPlayState(Console& console, Keyboard& keyboard, SoundManager& soundManager, StateMachine& stateMachine, std::unique_ptr<TetrisClient> client, uint64_t bagSeed)
    : m_Console(console)
    , m_Keyboard(keyboard)
    , m_SoundManager(soundManager)
    , m_StateMachine(stateMachine)
    , m_Client(std::move(client))
    , m_bagSeed(bagSeed)
{
    m_GameOverTimer = std::make_unique<Timer>();
    m_SoftDropTimer = std::make_unique<Timer>();
}

MultiPlayState::~MultiPlayState()
{
}

void MultiPlayState::CheckGameOverTransition()
{
    using Side = PlayerSide;

    if (m_Logic->IsGameOver(Side::Local))
    {
        // Local 패배
        m_bIsVictory = false;
    }
    else if (m_Logic->IsGameOver(Side::Remote))
    {
        // Local 승리
        m_bIsVictory = true;
    }
    else
    {
        return;
    }

    if (!m_bWaitingGameOverTransition)
    {
        m_GameOverTimer->Start();
        m_bWaitingGameOverTransition = true;
        m_SoundManager.StopBGM();

        if (m_bIsVictory)
            m_SoundManager.PlaySE_Force("victory");
        else
            m_SoundManager.PlaySE_Force("failure");
    }

    if (m_bWaitingGameOverTransition &&
        m_GameOverTimer->ElapsedMS() >= 3000)
    {
        m_bWaitingGameOverTransition = false;
        m_StateMachine.PushState(
            std::make_unique<GameOverState>(
                m_Console,
                m_Keyboard,
                m_SoundManager,
                m_StateMachine,
                m_bIsVictory
            )
        );
    }
}

void MultiPlayState::PlaySoundEffects()
{
    if (m_Logic->ShouldPlayFloorSE())
    {
        m_SoundManager.PlaySE_Force("floor");
    }

    if (m_Logic->ShouldPlayComboSE())
    {
        m_SoundManager.PlaySE_Force(m_Logic->GetComboString());
    }

    m_Logic->ClearSoundFlags();
}

void MultiPlayState::OnEnter()
{
    m_Console.ClearBuffer();
    m_SoundManager.PlayBGM("play_bgm");

    // --- Init Core Modules (Logic, Network, Renderer) ---
    m_Logic = std::make_unique<MultiPlayLogic>(m_bagSeed);
    m_Logic->Init();

    m_Network = std::make_unique<MultiPlayNetwork>(std::move(m_Client), *m_Logic);

    m_Renderer = std::make_unique<MultiPlayRenderer>(m_Console, *m_Logic);
    m_Renderer->InitLayout();


    m_SoftDropTimer->Start();
}

void MultiPlayState::OnExit()
{
    if (m_Network && m_Network->IsConnected())
    {
        m_Network->SendUnregister();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void MultiPlayState::Update()
{
    if (!m_Network->IsConnected())
    {
        TETRIS_LOG("Disconnected from server!");
        m_StateMachine.RequestPopDepth(1);
        return;
    }

    // --- Receive Packets (ASIO -> MainThread) ---
    m_Network->ProcessPackets();

    // --- Logic Step (Local Only) ---
    m_Logic->Update();


    PlaySoundEffects();
    

    // --- Send Packets (MainThread -> ASIO) ---
    m_Network->SyncToServer();

    // --- Game Over 처리 ---
    CheckGameOverTransition();
}

void MultiPlayState::Draw()
{
    m_Renderer->Draw();
}

void MultiPlayState::ProcessInputs()
{
    using Side = PlayerSide;

    if (m_Logic->IsGameOver(Side::Local))
        return;

    if (m_Keyboard.IsKeyJustPressed(KEY_LEFT))
    {
        if (m_Logic->TryMove(Side::Local, -1, 0))
        {
            m_SoundManager.PlaySE_Force("move");
        }
    }
    else if (m_Keyboard.IsKeyJustPressed(KEY_RIGHT))
    {
        if (m_Logic->TryMove(Side::Local, +1, 0))
        {
            m_SoundManager.PlaySE_Force("move");
        }
    }
    else if (m_Keyboard.IsKeyJustPressed(KEY_DOWN) || 
        (m_Keyboard.IsKeyHeld(KEY_DOWN) && m_SoftDropTimer->ElapsedMS() >= GameConfig::SoftDropIntervalMS))
    {
        if (m_Logic->TrySoftDrop(Side::Local))
        {
            m_SoundManager.PlaySE_Force("move");
            m_SoftDropTimer->Restart();
        }
    }
    else if (m_Keyboard.IsKeyJustPressed(KEY_UP))
    {
        m_Logic->TryRotateCW(Side::Local);
    }
    else if (m_Keyboard.IsKeyJustPressed(KEY_Z))
    {
        m_Logic->TryRotateCCW(Side::Local);
    }
    else if (m_Keyboard.IsKeyJustPressed(KEY_SPACE))
    {
        m_Logic->HardDrop(Side::Local);
        m_SoundManager.PlaySE_Force("harddrop");
    }
    else if (m_Keyboard.IsKeyJustPressed(KEY_C))
    {
        if (m_Logic->TryHold(Side::Local))
        {
            m_SoundManager.PlaySE_Force("hold");
        }
        
    }
}

bool MultiPlayState::Exit()
{
    return false;
}
