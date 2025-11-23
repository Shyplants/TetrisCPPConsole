#include "Tetromino.h"
#include <cassert>

Tetromino::Tetromino(Tetris::TetrominoType type) noexcept
	: m_Type{ type }
	, m_Rotation{ Tetris::Rotation::R0 }
{
}

void Tetromino::SetType(Tetris::TetrominoType type)
{
	m_Type = type;
}

void Tetromino::SetPos(int x, int y)
{
	m_Pos = { x,y };
}

void Tetromino::SetRotation(Tetris::Rotation r)
{
	m_Rotation = r;
}

void Tetromino::RotateCW()
{
	m_Rotation = Tetris::NextCW(m_Rotation);
}

void Tetromino::RotateCCW()
{
	m_Rotation = Tetris::NextCCW(m_Rotation);
}

const std::array<Vec2, MINO_COUNT> Tetromino::GetBlocks() const
{
	// [0...7)
	const int typeIndex = static_cast<size_t>(m_Type) - 1;
	assert((0 <= typeIndex && typeIndex < Tetris::MINO_TYPE_COUNT) && "Invalid typeIndex");

	return Shapes[typeIndex][static_cast<size_t>(m_Rotation)];
}

const int Tetromino::GetColor() const
{
	return ColorFromType(GetType());
}

//int Tetromino::ColorFromType(TetrominoType type)
//{
//	switch (type) {
//	case TetrominoType::I:
//		return LIGHT_BLUE;
//	case TetrominoType::O:
//		return YELLOW;
//	case TetrominoType::T:
//		return PURPLE;
//	case TetrominoType::L:
//		return LIGHT_RED;
//	case TetrominoType::J:
//		return BLUE;
//	case TetrominoType::S:
//		return GREEN;
//	case TetrominoType::Z:
//		return RED;
//	}
//
//	assert(false && "Unvalid TetrominoType");
//	
//	return -1;
//}

std::array<Vec2, MINO_COUNT> Tetromino::BlocksFromType(Tetris::TetrominoType type)
{
	// [0...7)
	const int typeIndex = static_cast<size_t>(type) - 1;
	assert((0 <= typeIndex && typeIndex < Tetris::MINO_TYPE_COUNT) && "Invalid typeIndex");

	return Shapes[typeIndex][0];
}

//constexpr Tetromino::Rotation Tetromino::nextCW(Rotation r)
//{
//	switch (r)
//	{
//	case Tetromino::Rotation::R0:
//		return Rotation::R90;
//	case Tetromino::Rotation::R90:
//		return Rotation::R180;
//	case Tetromino::Rotation::R180:
//		return Rotation::R270;
//	case Tetromino::Rotation::R270:
//		return Rotation::R0;
//	default:
//		assert(false && "SHOULD BE A VALID ROTATION TYPE");
//		return Rotation::R0;
//	}
//}

//constexpr Tetromino::Rotation Tetromino::nextCCW(Rotation r)
//{
//	switch (r)
//	{
//	case Tetromino::Rotation::R0:
//		return Rotation::R270;
//	case Tetromino::Rotation::R90:
//		return Rotation::R0;
//	case Tetromino::Rotation::R180:
//		return Rotation::R90;
//	case Tetromino::Rotation::R270:
//		return Rotation::R180;
//	default:
//		assert(false && "SHOULD BE A VALID ROTATION TYPE");
//		return Rotation::R0;
//	}
//}
