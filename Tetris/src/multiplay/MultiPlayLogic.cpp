#include "MultiPlayLogic.h"
#include "../Board.h"
#include "../BagRandom.h"
#include "../Tetromino.h"
#include "../Score.h"
#include "../utils/Timer.h"
#include "../utils/Logger.h"
#include "../GameConfig.h"

using namespace Tetris;

MultiPlayLogic::MultiPlayLogic(uint64_t bagSeed)
    : m_bagSeed(bagSeed)
{
    m_Board[0] = std::make_unique<Board>();
    m_Board[1] = std::make_unique<Board>();

    m_Bag[0] = std::make_unique<BagRandom>();
    m_Bag[1] = std::make_unique<BagRandom>();

    m_CurMino[0] = std::make_unique<Tetromino>();
    m_CurMino[1] = std::make_unique<Tetromino>();

    m_HoldType.fill(Tetris::TetrominoType::None);
    m_bGameOver.fill(false);

    m_Score = std::make_unique<Score>();

    m_GravityTimer = std::make_unique<Timer>();
    m_PlayTimer = std::make_unique<Timer>();
    m_ComboTimer = std::make_unique<Timer>();
}

MultiPlayLogic::~MultiPlayLogic() = default;

void MultiPlayLogic::Init()
{
    m_Bag[0]->Seed(m_bagSeed);
    m_Bag[1]->Seed(m_bagSeed);

    TrySpawnMino(PlayerSide::Local);
    TrySpawnMino(PlayerSide::Remote, true);

    UpdateGhost(PlayerSide::Local);
    UpdateGhost(PlayerSide::Remote);

    UpdatePreview(PlayerSide::Local);
    UpdatePreview(PlayerSide::Remote);

    m_GravityTimer->Start();
    m_SoftDropTimer->Start();
    m_PlayTimer->Start();
    m_ComboTimer->Start();
}

void MultiPlayLogic::Update()
{
    if (m_bGameOver[0] || m_bGameOver[1])
        return;

    if(m_GravityTimer->ElapsedMS() >= GravityIntervalMS())
    {
        if (!TryMove(PlayerSide::Local, 0, +1))
        {
            LockAndProceed(PlayerSide::Local);
        }
        else
        {
            m_GravityTimer->Restart();
        }
    }

    UpdateGhost(PlayerSide::Local);
    UpdatePreview(PlayerSide::Local);
}

// [Local Only]
bool MultiPlayLogic::TryMove(PlayerSide side, int dx, int dy)
{
    if (side != PlayerSide::Local)
    {
        TETRIS_ERROR("side must be Local Only");
        __debugbreak();
        return false;
    }

    int i = Idx(side);
    auto* cur = m_CurMino[i].get();

    if (!cur)
        return false;

    if (m_Board[i]->IsCollide(*cur, dx, dy))
        return false;

    cur->SetPos(cur->GetX() + dx, cur->GetY() + dy);
    m_bSyncCurMino = true;

    return true;
}

// [Local Only]
bool MultiPlayLogic::TryRotateCW(PlayerSide side)
{
    if (side != PlayerSide::Local)
    {
        TETRIS_ERROR("side must be Local Only");
        __debugbreak();
        return false;
    }

    int i = Idx(side);
    auto* cur = m_CurMino[i].get();

    if (!cur)
        return false;

    auto prev = cur->GetRotation();
    cur->RotateCW();

    if (m_Board[i]->IsCollide(*cur, 0, 0))
    {
        cur->SetRotation(prev);
        return false;
    }
    m_bSyncCurMino = true;

    return true;
}

// [Local Only]
bool MultiPlayLogic::TryRotateCCW(PlayerSide side)
{
    if (side != PlayerSide::Local)
    {
        TETRIS_ERROR("side must be Local Only");
        __debugbreak();
        return false;
    }

    int i = Idx(side);
    auto* cur = m_CurMino[i].get();

    if (!cur)
        return false;

    auto prev = cur->GetRotation();
    cur->RotateCCW();

    if (m_Board[i]->IsCollide(*cur, 0, 0))
    {
        cur->SetRotation(prev);
        return false;
    }
    m_bSyncCurMino = true;

    return true;
}

// [Local Only]
bool MultiPlayLogic::TryHold(PlayerSide side)
{
    if (side != PlayerSide::Local)
    {
        TETRIS_ERROR("side must be Local Only");
        __debugbreak();
        return false;
    }

    int i = Idx(side);
    if (m_bHasHeldThisTurn)
        return false;

    auto* cur = m_CurMino[i].get();
    if (!cur)
        return false;

    if (m_HoldType[i] == Tetris::TetrominoType::None)
    {
        m_HoldType[i] = cur->GetType();
        TrySpawnMino(side);
    }
    else
    {
        Tetromino tmp(m_HoldType[i]);
        tmp.SetPos(BOARD_WIDTH / 2, 1);

        if (m_Board[i]->IsCollide(tmp, 0, 0))
            return false;

        Tetris::TetrominoType oldType = cur->GetType();

        cur->SetType(m_HoldType[i]);
        cur->SetRotation(Tetris::Rotation::R0);
        cur->SetPos(BOARD_WIDTH / 2, 1);

        m_HoldType[i] = oldType;
    }

    m_bHasHeldThisTurn = true;
    m_bSyncHold = true;
    return true;
}

// [Local Only]
void MultiPlayLogic::HardDrop(PlayerSide side)
{
    if (side != PlayerSide::Local)
    {
        TETRIS_ERROR("side must be Local Only");
        __debugbreak();
        return;
    }

    int i = Idx(side);
    auto* cur = m_CurMino[i].get();

    int dropped = 0;
    while (TryMove(side, 0, 1))
        ++dropped;

    if (dropped > 0)
        m_Score->AddHardDrop(dropped);

    LockAndProceed(side);
}

// [Local Only]
bool MultiPlayLogic::TrySoftDrop(Tetris::PlayerSide side)
{
    if (side != PlayerSide::Local)
    {
        TETRIS_ERROR("side must be Local Only");
        __debugbreak();
        return false;
    }

    int i = Idx(side);
    auto* cur = m_CurMino[i].get();

    if (TryMove(side, 0, +1))
    {
        m_Score->AddSoftDrop(1);
        m_GravityTimer->Restart();
        return true;
    }

    return false;
}

void MultiPlayLogic::SetGameOver(PlayerSide side)
{
    m_bGameOver[Idx(side)] = true;
}

void MultiPlayLogic::ApplyEnemyMinoState(const sMinoState& state)
{
    int i = Idx(PlayerSide::Remote);

    auto* cur = m_CurMino[i].get();
    cur->SetType(static_cast<Tetris::TetrominoType>(state.type));
    cur->SetPos(state.x, state.y);
    cur->SetRotation(static_cast<Tetris::Rotation>(state.rot));

    UpdateGhost(PlayerSide::Remote);
}

void MultiPlayLogic::ApplyEnemyHoldState(Tetris::TetrominoType type)
{
    int i = Idx(PlayerSide::Remote);

    m_HoldType[i] = static_cast<Tetris::TetrominoType>(type);
}

void MultiPlayLogic::ApplyEnemyPreviewState(const sPreviewMinoState& state)
{
    int i = Idx(PlayerSide::Remote);

    for (int p = 0; p < Tetris::MINO_PREVIEW_COUNT; ++p)
    {
        m_PreviewMinos[i][p] = static_cast<Tetris::TetrominoType>(state.previewTypes[p]);
    }
}

void MultiPlayLogic::ApplyEnemyBoardState(const sBoardState& state)
{
    int i = Idx(PlayerSide::Remote);

    for (int c = 0; c < state.cells.size(); ++c)
    {
        int x = c % BOARD_WIDTH;
        int y = c / BOARD_WIDTH;

        m_Board[i]->Set(x, y, state.cells[c]);
    }
}

Board* MultiPlayLogic::GetBoard(PlayerSide side) const
{
    return m_Board[Idx(side)].get();
}

Tetromino* MultiPlayLogic::GetCurMino(PlayerSide side) const
{
    return m_CurMino[Idx(side)].get();
}

Tetromino* MultiPlayLogic::GetGhostMino(PlayerSide side) const
{
    return m_GhostMino[Idx(side)].get();
}

Tetris::TetrominoType MultiPlayLogic::GetHoldType(PlayerSide side) const
{
    return m_HoldType[Idx(side)];
}

const std::array<Tetris::TetrominoType, Tetris::MINO_PREVIEW_COUNT>& MultiPlayLogic::GetPreview(PlayerSide side) const
{
    return m_PreviewMinos[Idx(side)];
}

void MultiPlayLogic::ClearSyncFlags()
{
    m_bSyncCurMino = false;
    m_bSyncHold = false;
    m_bSyncPreview = false;
    m_bSyncBoard = false;
}

void MultiPlayLogic::ClearSoundFlags()
{
    m_bPlayFloorSE = false;
    m_bPlayComboSE = false;
}

// [Local Only]
bool MultiPlayLogic::TrySpawnMino(PlayerSide side, bool bInit)
{
    if (side != PlayerSide::Local && !bInit)
    {
        TETRIS_ERROR("side must be Local Only");
        __debugbreak();
        return false;
    }

    int i = Idx(side);

    Tetromino tempMino(m_Bag[i]->Peek(0));
    tempMino.SetPos(BOARD_WIDTH / 2, 1);

    if (m_Board[i]->IsCollide(tempMino, 0, 0))
    {
        SetGameOver(side);
        return false;
    }

    m_CurMino[i] = std::make_unique<Tetromino>(m_Bag[i]->Next());
    m_CurMino[i]->SetPos(BOARD_WIDTH / 2, 1);
    m_CurMino[i]->SetRotation(Tetris::Rotation::R0);

    UpdatePreview(side);
    return true;
}

// [Local Only]
void MultiPlayLogic::LockAndProceed(PlayerSide side)
{
    if (side != PlayerSide::Local)
    {
        TETRIS_ERROR("side must be Local Only");
        __debugbreak();
        return;
    }

    int i = Idx(side);
    auto* cur = m_CurMino[i].get();

    m_totalPieces++;

    m_Board[i]->Lock(*cur);
    int cleared = m_Board[i]->ClearFullLines();

    if (cleared > 0)
    {
        m_bShowCombo = true;
    }

    bool tSpin = false;
    m_Score->OnLinesCleared(cleared, tSpin);

    m_bHasHeldThisTurn = false;
    TrySpawnMino(side);

    m_bSyncBoard = true;
    m_bPlayFloorSE = true;
}

void MultiPlayLogic::UpdateGhost(PlayerSide side)
{
    int i = Idx(side);

    if (!m_CurMino[i])
        return;

    m_GhostMino[i] = std::make_unique<Tetromino>(*m_CurMino[i]);
    while (!m_Board[i]->IsCollide(*m_GhostMino[i], 0, +1))
        m_GhostMino[i]->SetPos(m_GhostMino[i]->GetX(), m_GhostMino[i]->GetY() + 1);
}

void MultiPlayLogic::UpdatePreview(PlayerSide side)
{
    int i = Idx(side);
    for (int p = 0; p < Tetris::MINO_PREVIEW_COUNT; ++p)
        m_PreviewMinos[i][p] = m_Bag[i]->Peek(p);

    if (side == PlayerSide::Local)
        m_bSyncPreview = true;
}

void MultiPlayLogic::OnComboAchieved(int combo)
{
    m_ComboString = "combo_" + std::to_string(combo);
    m_bPlayComboSE = true;
    m_bShowCombo = true;
}

int MultiPlayLogic::GravityIntervalMS() const
{
    int level = std::max(1, m_Score->GetLevel());
    int base = 700;
    int step = 50;
    int ms = base - (level - 1) * step;

    return std::max(80, ms);
}