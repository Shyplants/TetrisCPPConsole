#pragma once

#include "../common/TetrisTypes.h"
#include <memory>

class Console;
class ConsoleRenderer;
class MultiPlayLogic;

class MultiPlayRenderer
{
public:
    MultiPlayRenderer(Console& console, MultiPlayLogic& logic);
    ~MultiPlayRenderer();

    void InitLayout();
    void Draw();

private:
    void DrawPlayer(Tetris::PlayerSide side);

private:
    Console& m_Console;
    MultiPlayLogic& m_Logic;

    // 화면 배치 정보
    int m_ScreenWidth{};
    int m_ScreenHeight{};
    int m_CenterW{};
    int m_CenterH{};
    int m_BoardLeft{};
    int m_BoardTop{};

    int m_LastCombo{};

    // 로컬/원격 렌더러 분리
    std::unique_ptr<ConsoleRenderer> m_LocalRenderer;
    std::unique_ptr<ConsoleRenderer> m_RemoteRenderer;
};
