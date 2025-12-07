#pragma once

#include "./utils/Types.h"
#include "./utils/Colors.h"
#include "./common/TetrisTypes.h"
#include <array>

class Tetromino
{
public:
	explicit Tetromino(Tetris::TetrominoType type = Tetris::TetrominoType::None) noexcept;

	const Tetris::TetrominoType GetType() const { return m_Type; }
	const Tetris::Rotation GetRotation() const { return m_Rotation; }
	const int GetX() const { return m_Pos.x; }
	const int GetY() const { return m_Pos.y; }
	const Vec2 GetPos() const { return m_Pos; }

	void SetType(Tetris::TetrominoType type);
	void SetPos(int x, int y);
	void SetRotation(Tetris::Rotation r);

	void Rotate(bool cw);

	// 현재 회전 상태의 4블록 상대 좌표 반환
	const std::array<Vec2, Tetris::MINO_COUNT> GetBlocks() const;

	const std::array<Vec2, Tetris::MINO_COUNT> GetBlocks(Tetris::Rotation rotation) const;

	const int GetColor() const;

	// static int ColorFromType(TetrominoType type);

	// 미리보기 용도 상대 좌표 반환
	static std::array<Vec2, Tetris::MINO_COUNT> BlocksFromType(Tetris::TetrominoType type);

	static std::array<std::array< Vec2, Tetris::JLSTZ_OFFSET_COUNT>, Tetris::ROTATION_COUNT> Get_JLSTZ_OffsetData() { return JLSTZ_offsetData; }
	static std::array<std::array< Vec2, Tetris::I_OFFSET_COUNT>, Tetris::ROTATION_COUNT> Get_I_OffsetData() { return I_offsetData; }
	static std::array<std::array< Vec2, Tetris::O_OFFSET_COUNT>, Tetris::ROTATION_COUNT> Get_O_OffsetData() { return O_offsetData; }

private:
	// 시계/반시계 회전 (실제 격자 충돌 여부는 보드가 판단)
	// static constexpr Rotation nextCW(Rotation r);
	// static constexpr Rotation nextCCW(Rotation r);

	

private:
	// 각 미노의 4개 블록 상대좌표 (각 회전마다 4개 블록)
	static constexpr std::array<std::array<std::array<Vec2, Tetris::MINO_COUNT>, Tetris::ROTATION_COUNT>, Tetris::MINO_TYPE_COUNT> Shapes{{
		/* I */
		{{
			{{{-1,+0},{+0,+0},{+1,+0},{+2,+0}}}, /* R0 */
			{{{+0,-2},{+0,-1},{+0,+0},{+0,+1}}}, /* R90 */
			{{{-2,+0},{-1,+0},{+0,+0},{+1,+0}}}, /* R180 */
			{{{+0,-1},{+0,+0},{+0,+1},{+0,+2}}}, /* R270 */
		}},

		/* O */
		{{
			{{{+0,+0},{+1,+0},{+0,+1},{+1,+1}}}, /* R0 */
			{{{+0,-1},{+1,-1},{+0,+0},{+1,+0}}}, /* R90 */
			{{{-1,-1},{+0,-1},{-1,+0},{+0,+0}}}, /* R180 */
			{{{-1,+0},{+0,+0},{-1,+1},{+0,+1}}}, /* R270 */
		}},

		/* T */
		{{
			{{{-1,+0},{+0,+0},{+1,+0},{+0,+1}}}, /* R0 */
			{{{+0,-1},{+0,+0},{+1,+0},{+0,+1}}}, /* R90 */
			{{{+0,-1},{-1,+0},{+0,+0},{+1,+0}}}, /* R180 */
			{{{+0,-1},{-1,+0},{+0,+0},{+0,+1}}}, /* R270 */
		}},

		/* L */
		{{
			{{{-1,+0},{+0,+0},{+1,+0},{+1,+1}}}, /* R0 */
			{{{+0,-1},{+1,-1},{+0,+0},{+0,+1}}}, /* R90 */
			{{{-1,-1},{-1,+0},{+0,+0},{+1,+0}}}, /* R180 */
			{{{+0,-1},{+0,+0},{-1,+1},{+0,+1}}}, /* R270 */
		}},

		/* J */
		{{
			{{{-1,+0},{+0,+0},{+1,+0},{-1,+1}}}, /* R0 */
			{{{+0,-1},{+0,+0},{+0,+1},{+1,+1}}}, /* R90 */
			{{{+1,-1},{-1,+0},{+0,+0},{+1,+0}}}, /* R180 */
			{{{-1,-1},{+0,-1},{+0,+0},{+0,+1}}}, /* R270 */
		}},

		/* S */
		{{
			{{{-1,+0},{+0,+0},{+0,+1},{+1,+1}}}, /* R0 */
			{{{+1,-1},{+0,+0},{+1,+0},{+0,+1}}}, /* R90 */
			{{{-1,-1},{+0,-1},{+0,+0},{+1,+0}}}, /* R180 */
			{{{+0,-1},{-1,+0},{+0,+0},{-1,+1}}}, /* R270 */
		}},

		/* Z */
		{{
			{{{+0,+0},{+1,+0},{-1,+1},{+0,+1}}}, /* R0 */
			{{{+0,-1},{+0,+0},{+1,+0},{+1,+1}}}, /* R90 */
			{{{+0,-1},{+1,-1},{-1,+0},{+0,+0}}}, /* R180 */
			{{{-1,-1},{-1,+0},{+0,+0},{+0,+1}}}, /* R270 */
		}},
	}};

	static constexpr std::array<std::array< Vec2, Tetris::JLSTZ_OFFSET_COUNT>, Tetris::ROTATION_COUNT> JLSTZ_offsetData{{
		{{
			{+0, +0}, {+0, +0}, {+0, +0}, {+0, +0}, {+0, +0},
		}},
		{{
			{+0, +0}, {+1, +0}, {+1, -1}, {+0, +2}, {+1, +2},
		}},
		{{
			{+0, +0}, {+0, +0}, {+0, +0}, {+0, +0}, {+0, +0},
		}},
		{{
			{+0, +0}, {-1, +0}, {-1, -1}, {+0, +2}, {-1, +2},
		}},
	}};

	static constexpr std::array<std::array< Vec2, Tetris::I_OFFSET_COUNT>, Tetris::ROTATION_COUNT> I_offsetData{ {
		{{
			{+0, +0}, {-1, +0}, {+2, +0}, {-1, +0}, {+2, +0},
		}},
		{{
			{-1, +0}, {+0, +0}, {+0, +0}, {+0, +1}, {+0, -2},
		}},
		{{
			{-1, +1}, {+1, +1}, {-2, +1}, {+1, +0}, {-2, +0},
		}},
		{{
			{+0, +1}, {+0, +1}, {+0, +1}, {+0, -1}, {+0, +2},
		}},
	}};

	static constexpr std::array<std::array< Vec2, Tetris::O_OFFSET_COUNT>, Tetris::ROTATION_COUNT> O_offsetData{ {
		{{
			{+0, +0},
		}},
		{{
			{+0, -1},
		}},
		{{
			{-1, -1},
		}},
		{{
			{-1, +0},
		}},
	}};

private:
	Tetris::TetrominoType m_Type;
	Tetris::Rotation m_Rotation;
	Vec2 m_Pos{ 0, 0 };
};