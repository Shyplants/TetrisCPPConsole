#include "Score.h"

Score::Score()
{
}

void Score::Reset()
{
	m_Score = 0;
	m_Lines = 0;
	m_Level = 1;
	m_Combo = 0;
	m_bBackToBack = false;
}

void Score::OnLinesCleared(int cleared, bool isTSpin)
{
	if (cleared == 0)
	{
		m_Combo = 0;
		return;
	}

	// 라인 클리어 존재

	++m_Combo;
	AddLineScore(cleared, isTSpin);
	AddComboBonus(cleared);
	HandleBackToBack(isTSpin, cleared == 4);

	m_Lines += cleared;
	UpdateLevel();
}

void Score::AddSoftDrop(int cells)
{
	m_Score += cells;
}

void Score::AddHardDrop(int cells)
{
	m_Score += 2 * cells;
}

void Score::AddLineScore(int cleared, bool isTSpin)
{
	static constexpr int base[5] = { 0,100,300,500,800 };
	const int gain = base[cleared > 4 ? 4 : cleared] * m_Level;
	m_Score += gain;
}

void Score::AddComboBonus(int cleared)
{
	if (m_Combo <= 1)
		return;

	m_Score += 50 * (m_Combo - 1);
}

void Score::HandleBackToBack(bool isTSpin, bool isTetris)
{
	const bool bSpecial = isTSpin || isTetris;

	if (bSpecial)
	{
		if (m_bBackToBack)
		{
			// 백투백 보너스
			m_Score += 100 * m_Level;
		}
		m_bBackToBack = true;
	}
	else
		m_bBackToBack = false;
}

void Score::UpdateLevel()
{
	while (m_Lines >= m_Level * 10)
		++m_Level;
}
