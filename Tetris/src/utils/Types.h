#pragma once

#include <cstdint>

struct Vec2
{
	int x{}, y{};
};

inline const Vec2 operator+(const Vec2& lhs, const Vec2& rhs) {
	return Vec2({ lhs.x + rhs.x, lhs.y + rhs.y });
}

inline const Vec2 operator-(const Vec2& lhs, const Vec2& rhs) {
	return Vec2({ lhs.x - rhs.x, lhs.y - rhs.y });
}

constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_VISIBLE_HEIGHT = 20;
constexpr int BOARD_HIDDEN_HEIGHT = 3;
constexpr int BOARD_HEIGHT = BOARD_HIDDEN_HEIGHT + BOARD_VISIBLE_HEIGHT;

constexpr int ROTATION_COUNT = 4;
constexpr int MINO_COUNT = 4;
constexpr int MINO_PREVIEW_COUNT = 4;
constexpr int MINO_TYPE_COUNT = 7;