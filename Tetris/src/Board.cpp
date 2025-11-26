#include "Board.h"
#include "./utils/Logger.h"
#include <cassert>
#include "Console.h"
#include <string>
#include "common/PacketProtocol.h"
#include "Tetromino.h"

Board::Board(int width, int height) noexcept
	: m_Width{ width }
	, m_Height{ height }
	, m_Cells(static_cast<size_t>(width*height), 0)
{

}

Board::~Board() = default;

const int Board::Get(int x, int y) const
{
	if (OOB(x, y))
		return -1;

	return m_Cells[static_cast<size_t>(y * m_Width + x)];
}

void Board::Set(int x, int y, int val)
{
	if (OOB(x, y))
	{
		TETRIS_LOG("Unvalid params x: " + std::to_string(x) + "y: " + std::to_string(y));
		return;
	}

	if (val < 0 || val > m_TypeSize)
	{
		TETRIS_LOG("Unvalid params type: " + std::to_string(val));
		return;
	}

	m_Cells[static_cast<size_t>(y * m_Width + x)] = val;
}

void Board::Clear()
{
	std::fill(m_Cells.begin(), m_Cells.end(), 0);
}

const bool Board::IsCollide(const Tetromino& t, int dx, int dy, Tetris::Rotation rot) const
{
	auto blocks = GetBlocks(t.GetType(), rot);
	const int x0 = t.GetX() + dx;
	const int y0 = t.GetY() + dy;
	for (auto block : blocks)
	{
		const int x = x0 + block.x;
		const int y = y0 + block.y;
		if (OOB(x, y))
			return true; // 벽 or 바닥
		if (Get(x, y) != 0)
			return true; // 이미 다른 블록 존재
	}

	return false;
}

const bool Board::IsCollide(const Tetromino& t, int dx, int dy) const
{
	return IsCollide(t, dx, dy, t.GetRotation());
}

void Board::Lock(const Tetromino& t)
{
	const int color = static_cast<int>(t.GetType());
	auto blocks = t.GetBlocks();

	for (auto block : blocks) {
		const int x = t.GetX() + block.x;
		const int y = t.GetY() + block.y;

		Set(x, y, color);
	}
}

const int Board::ClearFullLines()
{
	int cleared = 0;
	for (int y = m_Height - 1; y >= 0; --y)
	{
		bool full = true;
		for (int x = 0; x < m_Width; ++x)
		{
			if (Get(x, y) == 0)
			{
				full = false;
				break;
			}
		}

		if (full)
		{
			++cleared;

			// 위에서 한줄 씩 내리기
			for (int yy = y; yy > 0; --yy)
			{
				for (int x = 0; x < m_Width; ++x)
					Set(x, yy, Get(x, yy - 1));
			}
			
			// 맨 위는 비우기
			for (int x = 0; x < m_Width; ++x)
				Set(x, 0, 0);

			++y; // 같은 y를 다시 검사(내려온 줄 검사)
		}
	}

	return cleared;
}

const int Board::GetCellColor(int x, int y) const
{
	if (OOB(x, y))
	{
		TETRIS_LOG("Unvalid params x: " + std::to_string(x) + "y: " + std::to_string(y));
		return -1;
	}

	auto minoType = Get(x, y);

	return Tetris::ColorFromType(static_cast<Tetris::TetrominoType>(minoType));
}

void Board::Draw(Console& console, int left, int top)
{
	
}

sBoardState Board::ToPacket() const
{
	sBoardState pkt;

	if (pkt.cells.size() != m_Cells.size())
	{
		TETRIS_ERROR("ToPacket Failed!");
		__debugbreak();
	}

	for (int i = 0; i < m_Cells.size(); ++i)
		pkt.cells[i] = m_Cells[i];

	return pkt;
}

std::array<Vec2, ROTATION_COUNT> Board::GetBlocks(Tetris::TetrominoType type, Tetris::Rotation rot)
{
	Tetromino temp{ type };
	temp.SetRotation(rot);
	return temp.GetBlocks();
}
