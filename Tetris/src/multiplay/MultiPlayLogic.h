#pragma once

#include "../common/TetrisTypes.h"
#include "../common/PacketProtocol.h"
#include <memory>
#include <array>

class Board;
class BagRandom;
class Score;
class Timer;
class Tetromino;

class MultiPlayLogic
{
public:
    MultiPlayLogic(uint64_t bagSeed);
    ~MultiPlayLogic();

    void Init();
    void Update();

    // --- 게임 조작 ---
    bool TryMove(Tetris::PlayerSide side, int dx, int dy);
    bool TryRotateCW(Tetris::PlayerSide side);
    bool TryRotateCCW(Tetris::PlayerSide side);
    bool TryHold(Tetris::PlayerSide side);
    void HardDrop(Tetris::PlayerSide side);
    bool TrySoftDrop(Tetris::PlayerSide side);

    // --- 상태 ---
    bool IsGameOver(Tetris::PlayerSide side) const { return m_bGameOver[(int)side]; }
    void SetGameOver(Tetris::PlayerSide side);

    int GravityIntervalMS() const;
    std::string GetComboString() const { return m_ComboString; }

    // --- 외부에서 서버로 전달된 상태 업데이트 ---
    void ApplyEnemyMinoState(const sMinoState& state);
    void ApplyEnemyHoldState(Tetris::TetrominoType type);
    void ApplyEnemyPreviewState(const sPreviewMinoState& state);
    void ApplyEnemyBoardState(const sBoardState& state);

    // --- 렌더링용 Getter ---
    Board* GetBoard(Tetris::PlayerSide side) const;
    Tetromino* GetCurMino(Tetris::PlayerSide side) const;
    Tetromino* GetGhostMino(Tetris::PlayerSide side) const;
    Tetris::TetrominoType GetHoldType(Tetris::PlayerSide side) const;
    const std::array<Tetris::TetrominoType, Tetris::MINO_PREVIEW_COUNT>&
        GetPreview(Tetris::PlayerSide side) const;

    const Score& GetScore() { return *m_Score; }
    const Timer& GetPlayTimer() { return *m_PlayTimer; }
    int GetTotalPieces() { return m_totalPieces; }
    bool& GetShowCombo() { return m_bShowCombo; }
    Timer& GetComboTimer() { return *m_ComboTimer; }

    bool ShouldSyncCurMino() const { return m_bSyncCurMino; }
    bool ShouldSyncHold() const { return m_bSyncHold; }
    bool ShouldSyncPreview() const { return m_bSyncPreview; }
    bool ShouldSyncBoard() const { return m_bSyncBoard; }

    bool ShouldPlayFloorSE() const { return m_bPlayFloorSE; }
    bool ShouldPlayComboSE() const { return m_bPlayComboSE; }

    void ClearSyncFlags();
    void ClearSoundFlags();

private:
    bool TrySpawnMino(Tetris::PlayerSide side, bool bInit = false);
    void LockAndProceed(Tetris::PlayerSide side);
    void UpdateGhost(Tetris::PlayerSide side);
    void UpdatePreview(Tetris::PlayerSide side);
    void OnComboAchieved(int combo);

    

private:
    int Idx(Tetris::PlayerSide side) const { return (side == Tetris::PlayerSide::Local) ? 0 : 1; }

private:
    // --- 게임 상태 ---
    std::array<std::unique_ptr<Board>, 2> m_Board;
    std::array<std::unique_ptr<BagRandom>, 2> m_Bag;
    std::array<std::unique_ptr<Tetromino>, 2> m_CurMino;
    std::array<std::unique_ptr<Tetromino>, 2> m_GhostMino;

    std::array< std::array<Tetris::TetrominoType, Tetris::MINO_PREVIEW_COUNT>, 2 > m_PreviewMinos;

    std::array<Tetris::TetrominoType, 2> m_HoldType;

    uint64_t m_bagSeed = 0;

    // --- Game State ---
    std::array<bool, 2> m_bGameOver;
    bool m_bHasHeldThisTurn{ false };
    std::string m_ComboString{};

    // --- 타이머/점수 ---
    std::unique_ptr<Score> m_Score;
    std::unique_ptr<Timer> m_GravityTimer;
    std::unique_ptr<Timer> m_PlayTimer;
    std::unique_ptr<Timer> m_ComboTimer;

    int m_totalPieces{ 0 };

    // --- 서버로 보내야 할 Sync Flags ---
    bool m_bSyncCurMino{ false };
    bool m_bSyncHold{ false };
    bool m_bSyncPreview{ false };
    bool m_bSyncBoard{ false };

    // --- 사운드 요청할 Event Flags ---
    bool m_bPlayFloorSE{ false };
    bool m_bPlayComboSE{ false };


    // --- 렌더러로 보내야 할 Sync Flags ---
    bool m_bShowCombo{ false };
};