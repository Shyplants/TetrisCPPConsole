#include "BagRandom.h"

BagRandom::BagRandom(uint64_t seed)
{
	Seed(seed);
}

Tetris::TetrominoType BagRandom::Next()
{
	if (m_Queue.empty())
		Refill();

	auto t = m_Queue.front();
	m_Queue.pop_front();
	
	return t;
}

const Tetris::TetrominoType BagRandom::Peek(size_t i)
{
	while (m_Queue.size() <= i)
		Refill();

	return m_Queue[i];
}

void BagRandom::Seed(uint64_t seed)
{
	m_Random.Reseed(seed);
	m_Queue.clear();
}

const void BagRandom::Refill()
{
	std::array<Tetris::TetrominoType, Tetris::MINO_TYPE_COUNT> bag{
		Tetris::TetrominoType::I,
		Tetris::TetrominoType::O,
		Tetris::TetrominoType::T,
		Tetris::TetrominoType::L,
		Tetris::TetrominoType::J,
		Tetris::TetrominoType::S,
		Tetris::TetrominoType::Z
	};

	std::shuffle(bag.begin(), bag.end(), m_Random.Engine());
	for (auto t : bag)
		m_Queue.push_back(t);
}


