#pragma once

#include <array>
#include <deque>
#include "./utils/Random.h"
#include <algorithm>
#include "./common/TetrisTypes.h"

// 7개의 미노를 셔플하여 제공
class BagRandom
{
public:
	BagRandom() = default;
	BagRandom(uint64_t seed);
	~BagRandom();

	// 다음 타입 하나 꺼내기 (없으면 자동 리필)
	Tetris::TetrominoType Next();

	// 미리보기
	const Tetris::TetrominoType Peek(size_t i = 0);

	// 큐 길이
	const size_t Size() const { return m_Queue.size(); }

	// 시드 설정
	void Seed(uint64_t seed);

private:
	// 7개의 미노 다시 채우기 (랜덤 적용)
	const void Refill();

private:
	Random m_Random{};
	mutable std::deque<Tetris::TetrominoType> m_Queue;
};