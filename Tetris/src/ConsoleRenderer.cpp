#include "ConsoleRenderer.h"
#include "Console.h"
#include "Board.h"
#include "BagRandom.h"
#include "Score.h"
#include "utils/Timer.h"
#include "Tetromino.h"
#include <cassert>

ConsoleRenderer::ConsoleRenderer(Console& console, int boardLeft, int boardTop)
	: m_Console{ console }
	, m_BoardLeft{ boardLeft }
	, m_BoardTop{ boardTop }
{

}

void ConsoleRenderer::DrawBoard(const Board& board, const Tetromino* curMino, const Tetromino* ghostMino)
{
	// 보드 경계 그리기
	m_Console.DrawHorz(m_BoardLeft - 2, m_BoardTop + BOARD_HEIGHT, BOARD_WIDTH + 2, LIGHT_YELLOW, L"█");
	m_Console.DrawVert(m_BoardLeft - 2, m_BoardTop + BOARD_HIDDEN_HEIGHT, BOARD_VISIBLE_HEIGHT, LIGHT_YELLOW, L"█");
	m_Console.DrawVert(m_BoardLeft + BOARD_WIDTH * 2, m_BoardTop + BOARD_HIDDEN_HEIGHT, BOARD_VISIBLE_HEIGHT, LIGHT_YELLOW, L"█");

	// 보드에 배치된 미노 그리기
	const int width = board.GetWidth();
	const int height = board.GetHeight();

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			auto minoType = board.Get(x, y);

			// (x,y)번째 칸에 미노의 블럭이 존재
			if (minoType > 0)
			{
				const int color = board.GetCellColor(x, y);
				assert(color > 0 && "color is not valid!");

				m_Console.Write(m_BoardLeft + x * 2, m_BoardTop + y, L"█", color);
			}
		}
	}

	// 고스트 미노
	if (ghostMino)
	{
		const int color = ghostMino->GetColor();
		for (auto b : ghostMino->GetBlocks())
		{
			int gx = ghostMino->GetX() + b.x;
			int gy = ghostMino->GetY() + b.y;
			if (!board.OOB(gx, gy))
			{
				m_Console.Write(m_BoardLeft + gx * 2, m_BoardTop + gy, L"▒", static_cast<WORD>(color));
			}
		}
	}

	// 현재 미노
	if (curMino)
	{
		int color = curMino->GetColor();
		for (auto b : curMino->GetBlocks())
		{
			int gx = curMino->GetX() + b.x;
			int gy = curMino->GetY() + b.y;
			if (!board.OOB(gx, gy))
			{
				m_Console.Write(m_BoardLeft + gx * 2, m_BoardTop + gy, L"█", static_cast<WORD>(color));
			}
		}
	}
}

void ConsoleRenderer::DrawHoldPanel(const Tetris::TetrominoType holdMinoType)
{
	const int panel_x = m_BoardLeft - 16;
	const int panel_y = m_BoardTop + BOARD_HIDDEN_HEIGHT;

	int current_y = panel_y;

	m_Console.Write(panel_x, current_y, L"HOLD");
	current_y += 1;

	int slotColors[8]{ 0 };
	if (holdMinoType == Tetris::TetrominoType::None)
	{
		// 홀드 미노가 없을 때
		for (int dy = 0; dy < 2; ++dy)
		{
			for (int dx = 0; dx < 4; ++dx)
			{
				m_Console.Write(panel_x + dx * 2, current_y + dy, L"█", BRIGHT_WHITE);
			}
		}
	}
	else
	{
		// 홀드 미노가 있을 때
		Draw4x2Mino(panel_x, current_y, Tetromino::BlocksFromType(holdMinoType), Tetris::ColorFromType(holdMinoType));
	}

	current_y += 2;
}

void ConsoleRenderer::DrawPreviewPanel(std::array<Tetris::TetrominoType, Tetris::MINO_PREVIEW_COUNT>& previewTypes)
{
	const int panel_x = m_BoardLeft + (BOARD_WIDTH + 1) * 2 + 2;
	const int panel_y = m_BoardTop + BOARD_HIDDEN_HEIGHT;

	m_Console.Write(panel_x, panel_y, L"NEXT");

	// 미리보기 미노(5개) 그리기
	for (int i = 0; i < Tetris::MINO_PREVIEW_COUNT; ++i)
	{
		const int cx = panel_x;
		const int cy = panel_y + 1 + 3 * (i + 1);

		int slotColors[8]{ 0 };

		auto type = previewTypes[i];
		const int color = Tetris::ColorFromType(type);
		auto blocks = Tetromino::BlocksFromType(type);

		for (auto b : blocks)
		{
			// b.x: [-1..3)
			// b.y: [0..2)
			slotColors[b.y * 4 + (b.x + 1)] = color;
		}

		for (int dy = 0; dy < 2; ++dy)
		{
			for (int dx = 0; dx < 4; ++dx)
			{
				const int slotIndex = dy * 4 + dx;
				m_Console.Write(cx + dx * 2, cy + dy, L"█", slotColors[slotIndex] == 0 ? BRIGHT_WHITE : slotColors[slotIndex]);
			}
		}
	}
}

void ConsoleRenderer::DrawInfoPanel(const Score& score, const Timer& playTimer, int totalPieces, int& lastCombo, bool& showCombo, Timer& comboTimer)
{
	const int panel_x = m_BoardLeft - 16;
	const int panel_y = m_BoardTop + 7 + BOARD_HIDDEN_HEIGHT;

	int current_y = panel_y;

	// 콤보 출력 (콤보 발생 후 1.5초 이내)
	static constexpr int COMBO_VISIBLE_MS = 1500;
	const int combo = score.GetCombo();
	bool bShowCombo = false;

	if (showCombo)
	{
		bShowCombo = true;
		lastCombo = combo;
		comboTimer.Restart();
		showCombo = false;
	}
	else if (lastCombo > 0) // 이번 턴에 콤보가 발생하지 않았지만 직전 콤보 출력여부 검사
	{
		if (comboTimer.ElapsedMS() < COMBO_VISIBLE_MS)
			bShowCombo = true;
		else
			lastCombo = 0; // 직전 콤보 출력 후 일정시간 지남
	}

	if (bShowCombo)
	{
		auto combo_string = std::to_wstring(lastCombo) + L"COMBO";
		m_Console.Write(panel_x, current_y, combo_string, GREEN);
	}
	current_y += 3;

	// 현재 레벨 출력
	auto level_string = std::to_wstring(score.GetLevel());
	m_Console.Write(panel_x, current_y, L"LEVEL", RED);
	m_Console.Write(panel_x, current_y + 1, level_string);
	current_y += 3;

	// 누적 피스갯수 출력
	auto pieces_string = std::to_wstring(totalPieces);
	m_Console.Write(panel_x, current_y, L"PIECES", RED);
	m_Console.Write(panel_x, current_y + 1, pieces_string);
	current_y += 3;

	// 경과시간 출력
	m_Console.Write(panel_x, current_y, L"TIME", RED);
	auto elapsedMS = playTimer.ElapsedMS();
	auto ms = elapsedMS % 1000;
	auto sec = elapsedMS / 1000 % 60;
	auto min = elapsedMS / 1000 / 60;

	auto ms_string = std::to_wstring(ms);
	auto sec_string = std::to_wstring(sec);
	auto min_string = std::to_wstring(min);

	if (sec_string.size() == 1)
		sec_string = L"0" + sec_string;

	if (ms_string.size() == 1)
		ms_string = L"00" + ms_string;
	else if (ms_string.size() == 2)
		ms_string = L"0" + ms_string;

	auto timeString = min_string + L":" + sec_string + L"." + ms_string;
	m_Console.Write(panel_x, current_y + 1, timeString);
	current_y += 3;

	// 점수 출력
	auto score_string = std::to_wstring(score.GetScore());
	auto needZeros = 8 - score_string.size();
	auto zero_string = std::wstring();
	for (int i = 0; i < needZeros; ++i)
		zero_string += L"0";

	score_string = zero_string + score_string;

	m_Console.Write(panel_x, current_y, L"SCORE", RED);
	m_Console.Write(panel_x, current_y + 1, score_string);
	current_y += 3;
}

void ConsoleRenderer::Draw4x2Mino(int x, int y, const std::array<Vec2, MINO_COUNT>& blocks, int color)
{
	int slot[8]{ 0 };
	for (auto b : blocks)
	{
		// b.x: [-1..3)
		// b.y: [0..2)

		int dx = b.x + 1;
		int dy = b.y;
		if (0 <= dx && dx < 4 && 0 <= dy && dy < 2)
		{
			slot[dy * 4 + dx] = color;
		}
		else
		{
			__debugbreak();
		}
	}

	for (int dy = 0; dy < 2; ++dy)
	{
		for (int dx = 0; dx < 4; ++dx)
		{
			int slotIndex = dy * 4 + dx;
			int drawColor = slot[slotIndex] ? slot[slotIndex] : BRIGHT_WHITE;
			m_Console.Write(x + dx * 2, y + dy, L"█", drawColor);
		}
	}
}

void ConsoleRenderer::Draw4x2Type(int x, int y, Tetris::TetrominoType type, int color)
{
}
