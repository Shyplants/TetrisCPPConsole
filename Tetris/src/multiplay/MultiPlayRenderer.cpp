#include "MultiPlayRenderer.h"
#include "../Console.h"
#include "../ConsoleRenderer.h"
#include "../Board.h"
#include "../Tetromino.h"
#include "../multiplay/MultiPlayLogic.h"
#include "../utils/Logger.h"

using namespace Tetris;

MultiPlayRenderer::MultiPlayRenderer(Console& console, MultiPlayLogic& logic)
    : m_Console(console)
    , m_Logic(logic)
{
    m_ScreenWidth = console.GetScreenWidth();
    m_ScreenHeight = console.GetScreenHeight();
    m_CenterW = console.GetHalfWidth();
    m_CenterH = console.GetHalfHeight();
}

MultiPlayRenderer::~MultiPlayRenderer() = default;

void MultiPlayRenderer::InitLayout()
{
    // 보드를 화면 가운데 기준으로 배치
    m_BoardLeft = m_CenterW - BOARD_WIDTH;
    m_BoardTop = m_CenterH - (BOARD_HEIGHT / 2);

    // 렌더러 생성 (왼쪽: Local, 오른쪽: Remote)
    m_LocalRenderer = std::make_unique<ConsoleRenderer>(m_Console, m_BoardLeft - 30, m_BoardTop);
    m_RemoteRenderer = std::make_unique<ConsoleRenderer>(m_Console, m_BoardLeft + 30, m_BoardTop);
}

void MultiPlayRenderer::Draw()
{
    m_Console.ClearBuffer();

    DrawPlayer(PlayerSide::Local);
    DrawPlayer(PlayerSide::Remote);
}

void MultiPlayRenderer::DrawPlayer(PlayerSide side)
{
    auto* renderer = (side == PlayerSide::Local) ? m_LocalRenderer.get() : m_RemoteRenderer.get();

    auto* board = m_Logic.GetBoard(side);
    auto* cur = m_Logic.GetCurMino(side);
    auto* ghost = m_Logic.GetGhostMino(side);
    auto  hold = m_Logic.GetHoldType(side);
    const auto& preview = m_Logic.GetPreview(side);

    if (!board)
        return;

    renderer->DrawBoard(*board, cur, ghost);
    renderer->DrawPreviewPanel(preview);
    renderer->DrawHoldPanel(hold);

    if (side == PlayerSide::Local)
    {
        renderer->DrawInfoPanel(m_Logic.GetScore(), m_Logic.GetPlayTimer(), m_Logic.GetTotalPieces(), m_LastCombo, m_Logic.GetShowCombo(), m_Logic.GetComboTimer());
    }
}
