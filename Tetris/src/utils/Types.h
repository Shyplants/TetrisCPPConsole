#pragma once

#include <cstdint>

struct Vec2
{
	int x{}, y{};
};

constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_VISIBLE_HEIGHT = 20;
constexpr int BOARD_HIDDEN_HEIGHT = 3;
constexpr int BOARD_HEIGHT = BOARD_HIDDEN_HEIGHT + BOARD_VISIBLE_HEIGHT;

constexpr int ROTATION_COUNT = 4;
constexpr int MINO_COUNT = 4;
constexpr int MINO_PREVIEW_COUNT = 4;
constexpr int MINO_TYPE_COUNT = 7;