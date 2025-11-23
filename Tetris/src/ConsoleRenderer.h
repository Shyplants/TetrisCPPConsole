#pragma once

#include <array>
#include "utils/Types.h"
#include "./common/TetrisTypes.h"

class Console;
class Board;
class BagRandom;
class Score;
class Timer;
class Tetromino;


class ConsoleRenderer
{
public:
	ConsoleRenderer(Console& console, int boardLeft, int boardTop);

	void DrawBoard(const Board& board, const Tetromino* curMino, const Tetromino* ghostMino);
	void DrawHoldPanel(const Tetris::TetrominoType holdMinoType);
	void DrawPreviewPanel(std::array<Tetris::TetrominoType, Tetris::MINO_PREVIEW_COUNT>& previewTypes);
	void DrawInfoPanel(const Score& score, const Timer& playTimer, int totalPieces, int& lastCombo, bool& showCombo, Timer& comboTimer);

private:
	void Draw4x2Mino(int x, int y, const std::array<Vec2, Tetris::MINO_COUNT>& blocks, int color);
	void Draw4x2Type(int x, int y, Tetris::TetrominoType type, int color);

private:
	Console& m_Console;
	int m_BoardLeft;
	int m_BoardTop;
};